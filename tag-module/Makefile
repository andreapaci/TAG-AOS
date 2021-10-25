CONFIG_MODULE_SIG=n

ifeq ($(KERNELRELEASE),)

.PHONY: all install clean uninstall
all:

	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
else
obj-m += TAGMOD.o
TAGMOD-objs += tag-module.o tag-syscall.o ../utils/hash-struct/hashmap.o ../utils/bitmask/bitmask.o
KBUILD_EXTRA_SYMBOLS := $(PWD)/../syscall-table-disc/Module.symvers

ccflags-y += -Wno-declaration-after-statement -Wno-implicit-fallthrough

ifeq ($(MOD_DEBUG), 1)
ccflags-y += -DAUDIT
endif
endif
