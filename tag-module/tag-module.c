/**
 *  @file   tag-module.c
 *  @brief  Source code for the module relative to TAG-based message exchange
 *  @author Andrea Paci
 */ 


#include "module.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea Paci <andrea.paci1998@gmail.com");
MODULE_DESCRIPTION("TAG-based message exchange");



static void initialize(void);

int init_module(void) {


    printk("%s: Mounting.\n", MODNAME);
    
    PRINT {
        printk("%s: Debug mode enabled ($MOD_DEBUG is set to 1)\n", MODNAME);
        printk("%s: Installing system calls\n", MODNAME);
    }

    install_syscalls();

    initialize();
    

    return 0;
}


static void initialize(void) {

    // Initialize TAG Table which maps "key" with "Tag Key" and the relative buffer
    tag_table = hashmap_new_with_allocator(
        kzalloc, 0, kfree, sizeof(tag_table_entry), 
        HASHMAP_CAP, SEED0, SEED1, 
        hashmap_murmur, tag_compare, 0);
    // SE NULL allora TERMINA
    
    //Initialize Bit Mask
    tag_bitmask = 0;

}










void cleanup_module(void) {
    printk("%s: Unmounting.\n", MODNAME);

}
