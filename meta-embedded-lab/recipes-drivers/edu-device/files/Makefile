ifneq ($(KERNELRELEASE),)
obj-m += edu_device.o
edu_device-objs := edu_main.o edu_chrdev.o edu_fifo.o edu_irq.o edu_hw.o edu_sysfs.o edu_debugfs.o edu_recovery.o edu_pm.o
else
ifneq ($(strip $(KERNEL_SRC)),)
KDIR := $(KERNEL_SRC)
else
KDIR ?= /lib/modules/$(shell uname -r)/build
endif
all:
	$(MAKE) -C $(KDIR) M=$$PWD modules
modules_install:
	$(MAKE) -C $(KDIR) M=$$PWD modules_install
clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
endif
