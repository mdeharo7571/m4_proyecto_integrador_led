obj-m += pwm.o 

export KROOT=/home/mdeharo/Documents/CTEP/MODULO_4/kernel/linux

allofit:  modules
modules: 
	@$(MAKE) -C $(KROOT) M=$(PWD) modules
modules_install:
	@$(MAKE) -C $(KROOT) M=$(PWD) modules_install		
kernel_clean:
	@$(MAKE) -C $(KROOT) M=$(PWD) clean

clean: kernel_clean
	rm -rf Module.symvers modules.order
