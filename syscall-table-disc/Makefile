CONFIG_MODULE_SIG=n

ifeq ($(KERNELRELEASE),)

.PHONY: all install clean uninstall
all:

	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
else
obj-m += SCTH.o
SCTH-objs += scth_module.o ptexplorer/ptexplorer.o arch-common/arch-common.o scth_discovery.o ../utils/hash-struct/hashmap.o ../utils/bitmask/bitmask.o

ccflags-y += -Wno-declaration-after-statement -Wno-implicit-fallthrough

ifeq ($(MOD_DEBUG), 1)
ccflags-y += -DAUDIT 
endif
ifeq ($(TEST_SYSCALL), 1)
ccflags-y += -DTEST_SYSCALL
endif
endif
