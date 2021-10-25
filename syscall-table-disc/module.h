#define EXPORT_SYMTAB
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/kprobes.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <asm/apic.h>
#include <linux/syscalls.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 0)
#error "Kernel must be at least version 4.17.x"
#endif

#ifdef AUDIT
#warning "Debug output Enabled"
#define PRINT if(1)
#else
#warning "Debug output Disabled"
#define PRINT if(0)
#endif


#define MODNAME "SCTH"