/**
 *  @file   tag-module.c
 *  @brief  Source code for the module relative to TAG-based message exchange
 *  @author Andrea Paci
 */ 


#include "module.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea Paci <andrea.paci1998@gmail.com");
MODULE_DESCRIPTION("TAG-based message exchange");

hashmap_t*           tag_table;
bitmask_t*           tag_bitmask;
tag_t**              tags;
struct rw_semaphore  common_lock; 

static int initialize(void);


int tag_compare(const void* a, const void* b, void* udata) {
    return ((tag_table_entry_t *) a) -> key - ((tag_table_entry_t *) b) -> key;
}

uint64_t tag_hash(const void *item, uint64_t seed0, uint64_t seed1 ) {
    const tag_table_entry_t* entry = item;
    return hashmap_sip( &(entry -> key), sizeof(int), seed0, seed1);
}



int init_module(void) {


    printk("%s: Mounting.\n", MODNAME);
    
    PRINT {
        printk("%s: Debug mode enabled ($MOD_DEBUG is set to 1)\n", MODNAME);
        printk("%s: Installing system calls\n", MODNAME);
    }
    
    if(initialize() != 0) {
        printk("%s: Error in initializing structs\n", MODNAME);
        return -1;
    }

    if(install_syscalls() == 0) {
        printk("%s: Error in installing system calls\n", MODNAME);

        kfree(tags);
        tags = 0;

        hashmap_free(tag_table);
        tag_table = 0;

        free_bitmask(tag_bitmask);
        tag_bitmask = 0;



        return -1;
    }

    

    return 0;
}


static int initialize(void) {

    // Initialize TAG Table which maps "key" with "Tag Key" and the relative buffer

    //NON VA BENE PECHE KZALLOC USA DUE PARAM
    // METTI BENE ERRORI RITORNO
    tag_table = hashmap_new_with_allocator(
        0, 0, 0, sizeof(tag_table_entry_t), 
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
        
        hashmap_free(tag_table);
        tag_table = 0;
        
        return -1;
    }

    // Initialize Tag pointer
    tags = kzalloc(sizeof(tag_t*) * MAX_TAGS, GFP_ATOMIC);
    if(tags == 0) {
        printk("%s: Error in creating TAG buffer\n", MODNAME);
        
        hashmap_free(tag_table);
        tag_table = 0;

        free_bitmask(tag_bitmask);
        tag_bitmask = 0;

        return -1;
    }

    init_rwsem(&common_lock);

    PRINT
    printk("%s: Struct initialized.\n", MODNAME);

    return 0;
}












void cleanup_module(void) {

    printk("%s: Unmounting.\n", MODNAME);
    
    // Check if address memory of the subsequent variable is avaliable to be freed or not
    // (Using kfree() on an unitialized address will result in not being able to unload the module)
    if(tag_bitmask != 0)
        free_bitmask(tag_bitmask);
    if(tag_table != 0)
        hashmap_free(tag_table);
    if(tags != 0)
        kfree(tags);
}
