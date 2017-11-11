ARCH=arm
CROSS_COMPILE=arm-none-linux-gnueabi-
obj-m := led_drv.o led_dev.o
# CC	= $(CROSS_COMPILE)gcc
KDIR	:=/root/qt_x210v3/kernel
PWD	:=$(shell pwd)
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	rm -rf *.o* *~  *.ko *.mod.c
	rm -rf *.tmp_versions/
	rm -rf *.symvers
