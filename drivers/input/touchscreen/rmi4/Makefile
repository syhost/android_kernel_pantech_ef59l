obj-$(CONFIG_RMI4_BUS) += rmi_bus.o
obj-$(CONFIG_RMI4_I2C) += rmi_i2c.o
#obj-$(CONFIG_RMI4_SPI) += rmi_spi.o
#obj-$(CONFIG_RMI4_SMB) += rmi_smbus.o
#obj-$(CONFIG_RMI4_GENERIC) += touch_fops.o touch_log.o rmi_driver.o rmi_f01.o
obj-$(CONFIG_RMI4_GENERIC) += rmi_driver.o rmi_f01.o
#obj-$(CONFIG_RMI4_FWLIB) += rmi_fw_update.o
#obj-$(CONFIG_RMI4_F09) += rmi_f09.o
obj-$(CONFIG_RMI4_F1A) += rmi_f1a.o
obj-$(CONFIG_RMI4_F11) += rmi_f11.o
#obj-$(CONFIG_RMI4_F17) += rmi_f17.o
#obj-$(CONFIG_RMI4_F19) += rmi_f19.o
#obj-$(CONFIG_RMI4_F21) += rmi_f21.o
#obj-$(CONFIG_RMI4_F30) += rmi_f30.o
#obj-$(CONFIG_RMI4_F34) += rmi_f34.o
#obj-$(CONFIG_RMI4_F41) += rmi_f41.o
#obj-$(CONFIG_RMI4_F54) += rmi_f54.o
#obj-$(CONFIG_RMI4_DEV) += rmi_dev.o

ANDROID_ROOT_DIR:=../../../../../
KDIR := $(ANDROID_ROOT_DIR)out/target/product/msm8974/obj/KERNEL_OBJ/
PWD := $(shell pwd)
ARCH:=arm
CROSS_COMPILE:=$(ANDROID_ROOT_DIR)prebuilt/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-

#EXTRA_CFLAGS += -include ../../../../../../../BOARD_REV.h

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -f -r *.o *.ko .*cmd *.mod.c .tmp* core *.i modules.order Module.symvers modules.builtin
