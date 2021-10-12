/**
 *  @file   tag-module.c
 *  @brief  Source code for the module relative to TAG-based message exchange
 *  @author Andrea Paci
 */ 


#include "module.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea Paci <andrea.paci1998@gmail.com");
MODULE_DESCRIPTION("TAG-based message exchange");

struct hashmap*  tag_table;
bitmask_struct*  tag_bitmask;
char**           tag_buffer;

static int initialize(void);







int init_module(void) {


    printk("%s: Mounting.\n", MODNAME);
    
    PRINT {
        printk("%s: Debug mode enabled ($MOD_DEBUG is set to 1)\n", MODNAME);
        printk("%s: Installing system calls\n", MODNAME);
    }

    if(install_syscalls() == 0) {
        printk("%s: Error in installing system calls\n", MODNAME);
        return -1;
    }

    if(initialize() == -1) {
        printk("%s: Error in initializing structs\n", MODNAME);
        return -1;
    }
    

    return 0;
}


static int initialize(void) {

    // Initialize TAG Table which maps "key" with "Tag Key" and the relative buffer


    //NON VA BENE PECHE KZALLOC USA DUE PARAM
    tag_table = hashmap_new_with_allocator(
        0, 0, 0, sizeof(tag_table_entry), 
        HASHMAP_CAP, SEED0, SEED1, 
        tag_hash, tag_compare, 0);
    if(tag_table == 0) {
         printk("%s: Error in creating TAG table\n", MODNAME);
         return -1;

    }
    
    // Initialize Bit Mask
    tag_bitmask = initialize_bitmask(MAX_TAGS);
    if(tag_bitmask == 0) {
        printk("%s: Error in creating TAG bitmask\n", MODNAME);
        return -1;
    }

    tag_buffer = kzalloc(MAX_TAGS * sizeof(char*), GFP_ATOMIC);
    if(tag_buffer == 0) {
        printk("%s: Error in creating TAG buffer\n", MODNAME);
        return -1;
    }

    PRINT
    printk("%s: Struct initialized.\n", MODNAME);

    return 0;
}












void cleanup_module(void) {

    printk("%s: Unmounting.\n", MODNAME);
    
    free_bitmask(tag_bitmask);
    hashmap_free(tag_table);
}
