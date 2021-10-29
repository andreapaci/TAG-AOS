/**
 *  @file   tag-module.c
 *  @brief  Source code for the module relative to TAG-based message exchange
 *  @author Andrea Paci
 */ 


#include "tag-dev-driver.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea Paci <andrea.paci1998@gmail.com");
MODULE_DESCRIPTION("TAG-based message exchange");

hashmap_t*           tag_table;
bitmask_t*           tag_bitmask;
tag_t**              tags;
struct rw_semaphore  common_lock;
struct rw_semaphore  tag_lock[MAX_TAGS];

int tag_get_nr;
int tag_send_nr;
int tag_receive_nr;
int tag_ctl_nr;


static int initialize(void);


int tag_compare(const void* a, const void* b, void* udata) {
    return ((tag_table_entry_t *) a) -> key - ((tag_table_entry_t *) b) -> key;
}

uint64_t tag_hash(const void *item, uint64_t seed0, uint64_t seed1 ) {
    const tag_table_entry_t* entry = item;
    return hashmap_sip( &(entry -> key), sizeof(int), seed0, seed1);
}



module_param(tag_get_nr,     int, S_IRUGO);
module_param(tag_send_nr,    int, S_IRUGO);
module_param(tag_receive_nr, int, S_IRUGO);
module_param(tag_ctl_nr,     int, S_IRUGO);

MODULE_PARM_DESC(tag_get_nr,     "tag_get() system call number");
MODULE_PARM_DESC(tag_send_nr,    "tag_send() system call number");
MODULE_PARM_DESC(tag_receive_nr, "tag_receive() system call number");
MODULE_PARM_DESC(tag_ctl_nr,     "tag_ctl() system call number");


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

    register_chardev();

    

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
    tags = kzalloc(sizeof(tag_t*) * MAX_TAGS, GFP_KERNEL);
    if(tags == 0) {
        printk("%s: Error in creating TAG buffer\n", MODNAME);
        
        hashmap_free(tag_table);
        tag_table = 0;

        free_bitmask(tag_bitmask);
        tag_bitmask = 0;

        return -1;
    }

    init_rwsem(&common_lock);

    int i;
    for(i = 0; i < MAX_TAGS; i++)
        init_rwsem(&(tag_lock[i]));
    

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
    if(tags != 0) {
        int i;
        // There's no risk in removing all instances of the Tag services since every system call increase the 
        // usage counter, so it's not possible to cleanup the module while using one of its system call
        for(i = 0; i < MAX_TAGS; i++) {
            if(tags[i] != 0) {
                clear_tag_level(tags[i] -> tag_level);
                kfree(tags[i] -> tag_level);
                kfree(tags[i]);
            }
        }

        kfree(tags);
    }

    unregister_chardev();
}
