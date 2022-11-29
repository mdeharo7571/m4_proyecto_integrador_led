#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/pwm.h>
#include <linux/delay.h>


/* Variables for device and device class */
static dev_t my_device_nr;
static struct class *my_class;
static struct cdev my_device;

#define DRIVER_NAME "my_pwm_driver"
#define DRIVER_CLASS "MyModuleClass"

/* Variables for pwm  */
struct pwm_device *pwm0 = NULL;

struct pwm_state *state;

u32 pwm_on_time = 1000000000;
u32 pwm_set_low = 0000000000;
u32 pwm_set_high= 100000000;



static ssize_t my_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta;
	char tmp[3] = " \n";

	/* Get amount of data to copy */
	to_copy = min(count, sizeof(tmp));

	/* Read value of pwm */
	//printk("Value of pwm: %d\n", pwm_get_state(pwm0,state));
	//tmp[0] = gpio_get_value(gpled) + '0';

	/* Copy data to user */
	not_copied = copy_to_user(user_buffer, &tmp, to_copy);

	/* Calculate data */
	delta = to_copy - not_copied;

	return delta;
}

static ssize_t my_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta;
	char value;

	/* Get amount of data to copy */
	to_copy = min(count, sizeof(value));

	/* Copy data to user */
	not_copied = copy_from_user(&value, user_buffer, to_copy);

	/* Setting the LED */
	/* Led On*/	
	if(value == '1'){
		pwm_enable(pwm0);
		pwm_config(pwm0, pwm_on_time, pwm_on_time);
		printk("Led on\n");
	}
	/* Led Off*/	
	if(value == '0'){
		pwm_disable(pwm0);
		pwm_config(pwm0,pwm_set_low, pwm_on_time);
		printk("Led off\n");
	}
	/* Led 10%*/	
	if(value == 'a')
	{
		pwm_enable(pwm0);
		pwm_config(pwm0,pwm_set_high, pwm_on_time);
		printk("10 percent\n");
	}

	if(value == 'b')
	{
		pwm_enable(pwm0);
		pwm_config(pwm0,pwm_set_high * 3, pwm_on_time);
		printk("30 percent\n");
	}
	if(value == 'c')
	{
		pwm_enable(pwm0);
		pwm_config(pwm0,pwm_set_high * 5, pwm_on_time);
		printk("50 percent\n");
	}

	if(value == 'd')
	{
		pwm_enable(pwm0);
		pwm_config(pwm0,pwm_set_high * 8, pwm_on_time);
		printk("80 percent\n");
	}

	if(value == 'e')
	{
		pwm_enable(pwm0);
		pwm_config(pwm0, pwm_on_time, pwm_on_time);
		printk("100 percent\n");
	}
		
	/* Calculate data */
	delta = to_copy - not_copied;

	return delta;
}


static int my_open(struct inode *device_file, struct file *instance){
	printk("Open was successful\n");
	return 0;
}


static int my_release(struct inode *device_file, struct file *instance){
	printk("Relase was successful\n");
	return 0;
}

/* file operations */
static struct file_operations fops = {
	.owner 	 = 	THIS_MODULE,
	.open 	 = 	my_open,
	.release = 	my_release,
	.read	 =	my_read,
	.write 	 = 	my_write
};

/* Menu option */
void led_menu_option(void){
	printk("\n");
	printk("----------------------------------------------\n");
	printk("		---Available options---	              \n");
	printk("	    	1-LED ON    0-LED OFF             \n");
	printk("    ----Porcentage intensity blinking-----    \n");
	printk("  a =10%% - b=30%% - c=50%% - d=80%%  e=100%% \n");
	printk("----------------------------------------------\n");
}

/* Blinking led when module is removed*/
void led_off_exit(void){
	int i=0;
   if(pwm_enable(pwm0)){
		pwm_disable(pwm0);
		pwm_config(pwm0, 000000000, pwm_on_time);
    	msleep(500);
    }
    while(i<=2){
		pwm_config(pwm0, pwm_on_time, pwm_on_time);
        msleep(1000);
	   	pwm_config(pwm0,000000000, pwm_on_time); 
       	msleep(1000);
       	i++;
    }
}

/* This function is called, when the module is loaded into the kernel*/
static int __init ModuleInit(void) {
	printk("Module Loaded!\n");
	led_menu_option();

	/* Allocate a device  */
	if( alloc_chrdev_region(&my_device_nr, 0, 1, DRIVER_NAME) < 0) {
		printk("Device could not be allocated!\n");
		return -1;
	}

	/* Create device class */
	if((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
		printk("Device class can not be created!\n");
		goto ClassError;
	}

	/* create device file */
	if(device_create(my_class, NULL, my_device_nr, NULL, DRIVER_NAME) == NULL) {
		printk("Can not create device file!\n");
		goto FileError;
	}

	/* Initialize device file */
	cdev_init(&my_device, &fops);

	/* Regisering device to kernel */
	if(cdev_add(&my_device, my_device_nr, 1) == -1) {
		printk("Registering of device to kernel failed!\n");
		goto AddError;
	}

	pwm0 = pwm_request(0, "my-pwm");
	if(pwm0 == NULL) {
		printk("Could not get PWM0!\n");
		goto AddError;
	}

	return 0;
AddError:
	device_destroy(my_class, my_device_nr);
FileError:
	class_destroy(my_class);
ClassError:
	unregister_chrdev_region(my_device_nr, 1);
	return -1;
}


/*This function is called, when the module is removed from the kernel*/
static void __exit ModuleExit(void) {
	pwm_disable(pwm0);
	pwm_free(pwm0);
	cdev_del(&my_device);
	device_destroy(my_class, my_device_nr);
	class_destroy(my_class);
	unregister_chrdev_region(my_device_nr, 1);
	led_off_exit();
	printk("Module Unloaded\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);

/* Module description */
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mariano Rafael De Haro Rodriguez");
MODULE_DESCRIPTION("Led PWM Module");
MODULE_INFO(board, "BeagleBone Black");


