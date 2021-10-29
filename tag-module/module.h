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
#include <linux/wait.h>
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
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/version.h>


#include "../syscall-table-disc/include/syscall-handle.h"
#include "../utils/include/bitmask.h"
#include "../utils/include/hashmap.h"
#include "tag-struct.h"

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



#define MODNAME     "TAG-MOD"
#define DEV_NAME    "tag_info"
// Size of the buffer used to read from the char device
#define CHAR_BLOCK  PAGE_SIZE

extern hashmap_t*           tag_table;
extern bitmask_t*           tag_bitmask;
extern tag_t**              tags;
extern struct rw_semaphore  common_lock;
extern struct rw_semaphore  tag_lock[MAX_TAGS];

// Offset of the installed syscall in the syscall table
extern int tag_get_nr;
extern int tag_send_nr;
extern int tag_receive_nr;
extern int tag_ctl_nr;


int install_syscalls(void);
void clear_tag_level(tag_level_t** tag_level);
