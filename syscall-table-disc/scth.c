/**
 *  @file   scth.c
 *  @brief  Source code for the module used for discovering the system call table position
 *          with its free entries at runtime 
 *  @author Andrea Paci
 */ 


#include "scth.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea Paci <andrea.paci1998@gmail.com");
MODULE_DESCRIPTION("System call table discovery and hacking");



// Start and End address for Kernel Address Space and 
#define START_ADDR		    ((unsiged long *) 0xffffffff00000000ULL)
#define END_ADDR		    ((unsiged long *) 0xfffffffffff00000ULL)

// Number of syscall entries in the table to analize
#define TABLE_ENTRIES       256

// Free entries in the syscall table (pointing at sys_ni_syscall)
#define FREE_ENTRIES        7
const char ni_syscall[] =	{ 134, 174, 182, 183, 214, 215, 236 };






int init_module(void) {

    printk("%s: Mounting.\n", MODNAME);
    PRINT
    printk("%s: Debug mode enabled ($MOD_DEBUG is set to 1)\n", MODNAME);

    get_phys_frame

    return 0;

}

void cleanup_module(void) {
    printk("%s: Unmounting.\n", MODNAME);
}