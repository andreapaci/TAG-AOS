CONFIG_MODULE_SIG=n

ifeq ($(KERNELRELEASE),)

.PHONY: all install clean uninstall
all:

	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
else
obj-m += TAGMOD.o
TAGMOD-objs += tag-module.o tag-syscall.o
KBUILD_EXTRA_SYMBOLS := $(PWD)/../syscall-table-disc/Module.symvers
ifeq ($(MOD_DEBUG), 1)
ccflags-y += -DAUDIT
endif
endif
