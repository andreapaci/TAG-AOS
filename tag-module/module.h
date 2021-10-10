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
#include <linux/gfp.h>

#include "../syscall-table-disc/include/syscall-handle.h"
#include "../utils/include/bitmask.h"
#include "../utils/include/hashmap.h"
#include "tag-struct.h"


#ifdef AUDIT
#define PRINT if(1)
#else
#define PRINT if(0)
#endif



#define MODNAME "TAG-MOD"

extern struct hashmap*  tag_table;
extern bitmask_struct*  tag_bitmask;
extern char**           tag_buffer;

int install_syscalls(void);