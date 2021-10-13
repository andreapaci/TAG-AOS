/**
 *  @file   tag-syscall.c
 *  @brief  Source code for the definition of the system calls relative to TAG module
 *  @author Andrea Paci
 */ 


#include "module.h"


int tag_get(int key, int command, int permission) {

    PRINT
    printk("%s: Tag get has been called.\n", MODNAME);

    // Impostare un try_lock che se fallisce (con stessa chiave) allora vuol dire che qualcun altro sta già creando lo stesso servizio (che si fa?)
    //Serializza l'operazioni su dati comuni come la bitmask
    if(key < 0) {
        PRINT
        printk("%s: Key is invalid (< 0)\n", MODNAME);
        return -EINVAL; 
    }

    // Creating new Tag Service
    if(command == TAG_CREAT) {

        // Check if more Tag services can be created 
        if(hashmap_count(tag_table) >= MAX_TAGS) {
            PRINT
            printk("%s: Maximum Tag services reached (%s)\n", MODNAME, MAX_TAGS);
            return -EMAXTAG;
        }

        // Get available tag number
        int tag_key; 


        //LOCK--------

        
        tag_key = get_avail_number(tag_bitmask);
        printk("%s: tag_key addr %p\n", MODNAME, &tag_key);
        if(tag_key < 0) {
            PRINT
            printk("%s: No tag_key avaliable\n", MODNAME);
            //UNLOCK--------
            return -EPROTO;
        }

        //if key IPC_PRIVATE no need to add it to the tag_table
        if(key != IPC_PRIVATE) {
            
            // Check if a Tag with the same key is already existing
            if(hashmap_get(tag_table, &(tag_table_entry_t){ .key = key}) != 0) {
                PRINT
                printk("%s: Tag with key %d already existing.\n", MODNAME, key);
                
                if(clear_number(tag_bitmask, tag_key) != 1)
                    printk("%s: Could not clear %d from bitmask.\n", MODNAME, tag_key);
                //UNLOCK--------
                return -EBUSY;
            }

            
            // Add new entry to the hashmap          
            if(hashmap_set(tag_table, &(tag_table_entry_t){ .key = key, .tag_key = tag_key}) == 0 && hashmap_oom(tag_table)) {
                PRINT
                printk("%s: Could not allocate hash struct entry.\n", MODNAME);
                
                if(clear_number(tag_bitmask, tag_key) != 1)
                    printk("%s: Could not clear %d from bitmask.\n", MODNAME, tag_key);
                //UNLOCK--------
                return -ENOMEM;
            }

            
        }

        //UNLOCK--------

        // Alloc TAG service buffer        
        tag_level_t* tag_level;
        tag_level = kzalloc(sizeof(tag_level_t) * LEVELS, GFP_ATOMIC);
        if(tag_level = 0) {
            PRINT
            printk("%s: Could not allocate memory for Tag Service levels.\n", MODNAME);
            return -ENOMEM;
        }


        tag_t* tag_entry = kzalloc(sizeof(tag_t), GFP_ATOMIC);
        if(tag_entry == 0) {
            PRINT
            printk("%s: Could not allocate memory for Tag Service entry.\n", MODNAME);
            return -ENOMEM;
        }


        
        tag_entry -> tag_key = tag_key;
        tag_entry -> permission = 0;
        tag_entry -> pid = current -> pid;
        tag_entry -> tag_level = tag_level;

        tag[tag_key] = *tag_entry;

        return tag_key;

    }
    else if(command == TAG_OPEN) {
        
        if(key == IPC_PRIVATE) {
            PRINT
            printk("%s: Key IPC_PRIVATE for open is invalid\n", MODNAME);
            return -EINVAL; 
        }

        // Check if a Tag with the same key is already existing
        tag_table_entry_t *entry;
        entry = hashmap_get(tag_table, &(tag_table_entry_t){ .key = key});
        
        if(entry == 0) {
            PRINT
            printk("%s: Tag with key %d does not exist.\n", MODNAME, key);
            return -ENODATA;
        }

        return entry -> tag_key;
        
    }
    
    PRINT
    printk("%s: Command is invalid\n", MODNAME);
    return -EINVAL; 
}


//NOTA--------------------------
//Inoltre, ricordati che quanti si fa tag_send e recv, fare un controllo preventivo se il buffer esiste
//inoltre, allo smontaggio, elimina tutte le entry
//Fai le FREE

int tag_send(int tag, int level, char* buffer, size_t size) { 

    PRINT
    printk("%s: Tag send has been called.\n", MODNAME);

    
    return 1;
}

int tag_receive(int tag, int level, char* buffer, size_t size) { 


    PRINT
    printk("%s: Tag receive has been called.\n", MODNAME);

    return 2;
}

int tag_ctl(int tag, int command) {

    PRINT
    printk("%s: Tag ctl has been called.\n", MODNAME);


    return 3;
}


__SYSCALL_DEFINEx(3, _tag_get, int, key, int, command, int, permission){ 
        return tag_get(key, command, permission);
}

unsigned long sys_tag_get;


__SYSCALL_DEFINEx(4, _tag_send, int, tag, int, level, char*, buffer, size_t, size) {
        return tag_send(tag, level, buffer, size);
}

unsigned long sys_tag_send;    


__SYSCALL_DEFINEx(4, _tag_receive, int, tag, int, level, char*, buffer, size_t, size) {
        return tag_receive(tag, level, buffer, size);
}

unsigned long sys_tag_receive;    


__SYSCALL_DEFINEx(2, _tag_ctl, int, tag, int, command) {
        return tag_ctl(tag, command);
}

unsigned long sys_tag_ctl;    


int install_syscalls(void) {
    
    sys_tag_get     = (unsigned long) __x64_sys_tag_get;
    sys_tag_send    = (unsigned long) __x64_sys_tag_send;
    sys_tag_receive = (unsigned long) __x64_sys_tag_receive;
    sys_tag_ctl     = (unsigned long) __x64_sys_tag_ctl;

    return
    syscall_insert((unsigned long *) sys_tag_get)       *
    syscall_insert((unsigned long *) sys_tag_send)      *
    syscall_insert((unsigned long *) sys_tag_receive)   *
    syscall_insert((unsigned long *) sys_tag_ctl);

}






