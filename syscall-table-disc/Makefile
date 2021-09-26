CONFIG_MODULE_SIG=n

ifeq ($(KERNELRELEASE),)

.PHONY: all install clean uninstall
all:

	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
else
obj-m += SCTH.o
SCTH-objs += scth.o ptexplorer.o arch-common.o scth_disc.o

ifeq ($(MOD_DEBUG), 1)
ccflags-y += -DAUDIT
ifeq ($(TEST_SYSCALL), 1)
ccflags-y += -DTEST_SYSCALL
endif
endif
endif
