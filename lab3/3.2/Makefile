CONFIG_MODULE_SIG=n
ifneq ($(KERNELRELEASE),)
obj-m := chardev_drive.o # 编译成外部模块
else
KERNELDIR := /lib/modules/$(shell uname -r)/build # 定义一个变量，指向内核目录
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules # 编译内核模块

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean # 清理编译产生的文件

all : test
.PHONY : all
test: test_char_device.c
	gcc -o $@ $<

endif