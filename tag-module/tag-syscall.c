/**
 *  @file   tag-syscall.c
 *  @brief  Source code for the definition of the system calls relative to TAG module
 *  @author Andrea Paci
 */ 


#include "module.h"

static int  add_tag_level(tag_level_t** tag_level);
static tag_level_t* create_level(int i, int epoch);
static int clear_tag_common(int key, int tag_key);
__always_inline static void free_level(tag_level_t* tag_level);
static void print_tag(void);
static void print_level(tag_level_t* tag_level, int tag);

/**
 *  @brief  Create or open a new Tag
 *  
 *  @param  key used for identify the Tag
 *  @param  command used to determine if is a "open" or a "create"
 *  @param  permission to enable the tag to be used by all threads 
 *          or only by the one of the same user who created the TAG
 * 
 *  @return Tag descriptor value used to reference the Tag or ERRNO code to signal error
 */
int tag_get(int key, int command, int permission) {

    PRINT {
        
        char* command_str;
        char* perm_str;
        
        if(command == TAG_OPEN)         command_str = "TAG_OPEN";
        else if(command == TAG_CREAT)   command_str = "TAG_CREAT";
        else                            command_str = "UNDEFINED";

        if(permission == TAG_PERM_USR)      perm_str = "TAG_PERM_USR";
        else if(permission == TAG_PERM_ALL) perm_str = "TAG_PERM_ALL";
        else                                perm_str = "UNDEFINED";

        printk("%s: TAG_GET called. TID: %d, key %d, command %s, perm: %s\n", 
            MODNAME, current->pid, key, command_str, perm_str);
    }
    
    
    if(key < 0) {
        PRINT
        printk("%s: Key is invalid (< 0)\n", MODNAME);
        return -EINVAL; 
    }


    // Create new Tag service
    if(command == TAG_CREAT) {

        // Check correct usage of the permission parameter
        if(permission != TAG_PERM_USR && permission != TAG_PERM_ALL) {
            PRINT
            printk("%s: Permission invalid.\n", MODNAME);
            return -EINVAL;
        }
        
        // Access common lock in write mode. This is necessary because common data structure will be accessed
        //  - Hashamp (containing the mapping [key -> tag descriptor])
        //  - Bitmask (for retriving the first avaliable tag descriptor value)
        // Those structs are not safe to use in parallel, in particular the Hashmap because of the bucket resizing (memory allocation/deallocation)
        // The bitmask could theoretically be accessed in concurrency using the "atmomic" operation on bits, but since the only access to it
        // is next to the one made for the hashmap (even for deleting a tag), there's no real performance boost in enabling concurrent
        // access (also considering the access to the bitmask is fast enough)
        if(unlikely(down_write_killable(&common_lock) == -EINTR)) {
            PRINT
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            return -EINTR;
        }
        
        // Check if more Tag services can be created 
        if(hashmap_count(tag_table) >= MAX_TAGS) {
            PRINT
            printk("%s: Maximum Tag services reached (%d)\n", MODNAME, MAX_TAGS);
            up_write(&common_lock);
            return -EMAXTAG;
        }

        // Get available Tag descriptor number
        int tag_key; 
        tag_key = get_avail_number(tag_bitmask);
        if(unlikely(tag_key < 0 || tag_key >= MAX_TAGS)) {
            PRINT
            printk("%s: No tag_key avaliable\n", MODNAME);
            up_write(&common_lock);
            return -EPROTO;
        }

        // If key IPC_PRIVATE no need to add it to the tag_table hashmap 
        // (it will never be necessary to get key -> tag descriptor mapping)
        if(key != IPC_PRIVATE) {
            
            // Check if a Tag with the same key is already existing
            if(hashmap_get(tag_table, &(tag_table_entry_t){ .key = key}) != 0) {
                PRINT
                printk("%s: Tag with key %d already existing.\n", MODNAME, key);
                if(clear_number(tag_bitmask, tag_key) != 1) 
                    PRINT
                    printk("%s: Critical error! Could not clear %d from bitmask.\n", MODNAME, tag_key);
                up_write(&common_lock);
                return -EBUSY;
            }
            
            // Add new entry to the Hashmap          
            if(unlikely(
                hashmap_set(tag_table, &(tag_table_entry_t){ .key = key, .tag_key = tag_key}) == 0 && 
                hashmap_oom(tag_table))) { //Translated in: if it fails to allocate data for the Hashamp entry
                
                PRINT
                printk("%s: Could not allocate hash struct entry.\n", MODNAME);
                if(clear_number(tag_bitmask, tag_key) != 1)
                    PRINT
                    printk("%s: Critical error! Could not clear %d from bitmask.\n", MODNAME, tag_key);
                up_write(&common_lock);
                return -ENOMEM;
            }

        }

        // No write on common struct needed for now, the subsequent code will only allocate struct used to represent
        // the new tag service (beside on a failure of these allocatation that requires freeing the common data structures)
        up_write(&common_lock);


        // Alloc TAG Levels buffer        
        tag_level_t** tag_level;
        tag_level = kzalloc(sizeof(tag_level_t*) * LEVELS, GFP_KERNEL);
        if(unlikely(tag_level == 0)) {
            PRINT
            printk("%s: Could not allocate memory for Tag Service levels array.\n", MODNAME);
            if(unlikely(clear_tag_common(key, tag_key) != 0)) return -EINTR;
            return -ENOMEM;
        }
        
        
        // Allocate and add all levels to tag_level
        if(add_tag_level(tag_level) != 0) {
            PRINT
            printk("%s: Could not allocate memory for Tag Service single levels.\n", MODNAME);
            
            // Free the level that have been allocated
            clear_tag_level(tag_level);
            kfree(tag_level);
            if(unlikely(clear_tag_common(key, tag_key) != 0)) return -EINTR;
            return -ENOMEM;
        }

        tag_t* tag_entry;
        tag_entry = kzalloc(sizeof(tag_t), GFP_KERNEL);
        if(unlikely(tag_entry == 0)) {
            PRINT
            printk("%s: Could not allocate memory for Tag Service entry.\n", MODNAME);

            // Free the level that have been allocated
            clear_tag_level(tag_level);
            kfree(tag_level);
            if(unlikely(clear_tag_common(key, tag_key) != 0)) return -EINTR;
            return -ENOMEM;
        }

        int i;
        
        // Initalize values for tag entry
        tag_entry -> key        = key;
        tag_entry -> tag_key    = tag_key;
        tag_entry -> ready      = 0;
        tag_entry -> permission = permission;
        tag_entry -> euid       = current_euid().val;
        tag_entry -> tag_level  = tag_level;
        atomic_set(&(tag_entry -> waiting), 0);
        for(i = 0; i < LEVELS; i++) init_rwsem(&(tag_entry -> level_lock[i]));
        
        // It's not necessary to lock this access because of the locking mechanism before:
        //      it's not possible to use an already taken tag descriptor (tag_key)
        //      Moreover, if a concurrent TAG CTL with DELETE gets called, it will have no effect until
        //      it will find the tag_entry in tags[tag_key], so no need to serialize this piece of code
        tags[tag_key] = tag_entry;

        PRINT
        print_tag();

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
        int tag_key;

        if(unlikely(down_read_interruptible(&common_lock) == -EINTR)) {
            PRINT
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            return -EINTR;
        }

        entry = hashmap_get(tag_table, &(tag_table_entry_t){ .key = key});
        if(entry == 0) {
            PRINT
            printk("%s: Tag with key %d does not exist.\n", MODNAME, key);
            up_read(&common_lock);
            return -ENODATA;
        }

        tag_key = entry -> tag_key;
        if(unlikely(tags[tag_key] == 0)) {
            PRINT
            printk("%s: Tag with tag descriptor %d is being deleted or is not yet fully initialized.\n", MODNAME, tag_key);
            up_read(&common_lock);
            return -ENODATA;
        }

        up_read(&common_lock); 

        PRINT
        print_tag();

        return tag_key;
    }
    

    PRINT
    printk("%s: Command is invalid\n", MODNAME);
    return -EINVAL; 
}


/**
 *  @brief  Send message to a Tag
 *  
 *  @param  tag Tag descriptor of the Tag
 *  @param  level of the Tag send message to (0 to LEVELS - 1)
 *  @param  buffer containing the message to deliver 
 *  @param  size size of the message to deliver
 * 
 *  @return 1 on success, 0 on discarded message (no receiver waiting or occupied), negative error codes otherwise
 */
int tag_send(int tag, int level, char* buffer, size_t size) { 


    PRINT
    printk("%s: TAG_SEND called. TID: %d, tag %d, level %d, buffer: %s, size: %ld\n", MODNAME, current->pid, tag, level, buffer, size);

    // Input check (buffer == 0 is permitted if the thread just want to wake up reaceiving thread)
    if(tag < 0 || tag >= MAX_TAGS || level < 0 || level >= LEVELS || size < 0 || size > BUFFER_SIZE){
        PRINT
        printk("%s: TAG_SEND: Wrong parameter usage\n", MODNAME);
        return -EINVAL;
    }

    if(buffer == 0) size = 0;


    // Get lock to access the (used to avoid removal while accessing the TAG)
    if(unlikely(down_read_interruptible(&(tag_lock[tag])) == -EINTR)) {  
        PRINT              
        printk("%s: RW Lock was interrupted.\n", MODNAME);
        return -EINTR;
    }

    tag_t* tag_entry;
    tag_entry = tags[tag];

    if(tag_entry == 0) {
        PRINT
        printk("%s: Tag %d is not created.\b", MODNAME, tag);
        up_read(&(tag_lock[tag]));
        return -ENODATA;
    }

    // Root can always access
    if(CHECKPERM(tag_entry)) {
        PRINT
        printk("%s: Could not access the Tag service %d: permission error\n", MODNAME, tag);
        up_read(&(tag_lock[tag]));
        return -EPERM;
    }
    
    if(atomic_read(&(tag_entry -> waiting)) == 0) {
        PRINT
        printk("%s: Tag %d has no reader.\b", MODNAME, tag);
        up_read(&(tag_lock[tag]));
        return 0;
    }

    if(unlikely(down_read_interruptible(&(tag_entry -> level_lock[level])) == -EINTR)) {                
        PRINT
        printk("%s: RW Lock was interrupted.\n", MODNAME);
        up_read(&(tag_lock[tag]));
        return -EINTR;
    }

    tag_level_t* tag_level;
    tag_level = (tag_entry -> tag_level)[level];

    if(unlikely(tag_level == 0)) {
        PRINT
        printk("%s: Tag %d with level %d is not existing.\n", MODNAME, tag, level);
        up_read(&(tag_entry -> level_lock[level]));
        up_read(&(tag_lock[tag]));
        return -EINTR;
    }


    if(unlikely(down_read_interruptible(&(tag_level -> rcu_lock)) == -EINTR)) {                
        printk("%s: RW Lock was interrupted.\n", MODNAME);
        up_read(&(tag_entry -> level_lock[level]));
        up_read(&(tag_lock[tag]));
        return -EINTR;
    }

    up_read(&(tag_entry -> level_lock[level]));
    
    // Try to acquire mutex (if fails, it means some else is writing)
    if(!mutex_trylock(&(tag_level -> w_mutex))) {
        PRINT
        printk("%s: Tag %d on level %d is contended/occupied.\b", MODNAME, tag, level);
        up_read(&(tag_level -> rcu_lock));
        up_read(&(tag_lock[tag]));
        return 0;
    }


    //Those next two "if" are separted to print distinguished info for the two cases
    if((tag_level -> ready) == 1) {
        PRINT
        printk("%s: Tag %d on level %d is occupied.\b", MODNAME, tag, level);
        mutex_unlock(&(tag_level -> w_mutex));
        up_read(&(tag_level -> rcu_lock));
        up_read(&(tag_lock[tag]));
        return 0;
    }

    if(atomic_read(&(tag_level -> waiting)) == 0) {
        PRINT
        printk("%s: Tag %d on level %d has no reader.\b", MODNAME, tag, level);
        mutex_unlock(&(tag_level -> w_mutex));
        up_read(&(tag_level -> rcu_lock));
        up_read(&(tag_lock[tag]));
        return 0;
    }


    // Only if size is > 0 the copy goes on, otherwise, just wake up
    if(size > 0) {
        // Copy of the buffer
        if(unlikely(copy_from_user(tag_level -> buffer, buffer, size) != 0)) {
            PRINT
            printk("%s: Error in copying message from userspace\n", MODNAME);
            up_read(&(tag_level -> rcu_lock));
            up_read(&(tag_lock[tag]));
            mutex_unlock(&(tag_level -> w_mutex));
            return -EFAULT;
        }
    }

    tag_level -> size = size;
    asm volatile("mfence" ::: "memory");

    PRINT
    print_level(tag_level, tag);
    
    // This will also prevent other senders to overwirte the buffer
    tag_level -> ready = 1;
    
    mutex_unlock(&(tag_level -> w_mutex));
    
    wake_up_all(&(tag_level -> local_wq));

    up_read(&(tag_level -> rcu_lock));
    up_read(&(tag_lock[tag]));

    PRINT
    printk("%s: TAG_SEND done. TID: %d, tag %d, level %d\n", MODNAME, current->pid, tag, level);
    
    return 1;
}









/**
 *  @brief  Receive message from a Tag
 *  
 *  @param  tag Tag descriptor of the Tag
 *  @param  level of the Tag send message to (0 to LEVELS - 1)
 *  @param  buffer memory position to store the message 
 *  @param  size size of the buffer
 * 
 *  @return 1 on success, 0 if interrupted while waiting or Awake_All, negative error codes otherwise
 */
int tag_receive(int tag, int level, char* buffer, size_t size) { 

    PRINT
    printk("%s: TAG_RECEIVE called. TID: %d, tag %d, level %d, size: %ld\n", MODNAME, current->pid, tag, level, size);

    int return_code;

    // Input check (buffer == NULL is allowed in case a thread just want to be woken up)
    if(tag < 0 || tag >= MAX_TAGS || level < 0 || level >= LEVELS || size < 0 || size > BUFFER_SIZE){
        PRINT
        printk("%s: TAG_RECEIVE Wrong parameter usage\n", MODNAME);
        return -EINVAL;
    }

    if(buffer == 0) size = 0;


    if(unlikely(down_read_interruptible(&(tag_lock[tag])) == -EINTR)) {                
            PRINT
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            return -EINTR;
    }

    tag_t* tag_entry;
    tag_entry = tags[tag];

    if(tag_entry == 0) {
        PRINT
        printk("%s: Tag %d is not created.\b", MODNAME, tag);
        up_read(&(tag_lock[tag]));
        return -ENODATA;
    }


    if(CHECKPERM(tag_entry)) {
        PRINT
        printk("%s: Could not access the Tag service %d: permission error\n", MODNAME, tag);
        up_read(&(tag_lock[tag]));
        return -EPERM;
    }
    
    if(unlikely(down_read_interruptible(&(tag_entry -> level_lock[level])) == -EINTR)) {                
        PRINT
        printk("%s: RW Lock was interrupted.\n", MODNAME);
        up_read(&(tag_lock[tag]));
        return -EINTR;
    }


    tag_level_t* tag_level;
    tag_level = (tag_entry -> tag_level)[level];


    if(unlikely(tag_level == 0)) {
        PRINT
        printk("%s: Tag %d with level %d is not existing.\n", MODNAME, tag, level);
        up_read(&(tag_entry -> level_lock[level]));
        up_read(&(tag_lock[tag]));
        return -EINVAL;
    }

    atomic_inc(&(tag_entry -> waiting));

    if(unlikely(down_read_interruptible(&(tag_level -> rcu_lock)) == -EINTR)) {                
        PRINT
        printk("%s: RW Lock was interrupted.\n", MODNAME);

        if(atomic_dec_and_test(&(tag_entry -> waiting))) 
            tag_entry -> ready = 0;
        up_read(&(tag_entry -> level_lock[level]));
        up_read(&(tag_lock[tag]));
        return -EINTR;
    }

    up_read(&(tag_entry -> level_lock[level]));
    
    // Important: if in this frame of time a new epoch level gets created (leaving this thread with an old version) there's is no problem
    // since it will enter in the next if, and will get as tag_level the updated version (new epoch one)



    // If the specified tag level has a send already, create a new epoch level and register on that one
    if(tag_level -> ready == 1) {
        PRINT
        printk("%s: Creating new Level Epoch (Tag: %d, Level: %d)\n", MODNAME, tag, level);

        tag_level_t* old_level;
        
        // Lock level structure
        // Lock has been taken here to avoid locking every time a receive occurs (since is not necessary), resulting
        // in redoing the same check on "read == 1" in case two or more thread starts a receive while ready is set to one
        if(unlikely(down_write_killable(&(tag_entry -> level_lock[level])) == -EINTR)) {                
            PRINT
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            
            if(atomic_dec_and_test(&(tag_entry -> waiting))) 
                tag_entry -> ready = 0;
            up_read(&(tag_level -> rcu_lock));
            up_read(&(tag_lock[tag]));
            
            return -EINTR;
        }

        // Save the old RCU lock
        struct rw_semaphore* temp_sem;
        temp_sem = &(tag_level -> rcu_lock);

        // In case multiple thread read the current level as "ready", they will both try to create new level and access it, 
        // so let's re-do the check and re-access the level to see if the thread is the first one or the subsequent
        // Instead of this, it could have been done using a write_try_lock instead of a lock, and then a subsequent write_lock to syncronize 
        // (to wait for the thread that started the allocation of a new level)
        // Every one accessing this specific level have to wait this routine to end (even in read) because the pointer to the level will
        // be changed, and so need those thread need to see the correct version of it
        
        
        tag_level = (tag_entry -> tag_level)[level];
        old_level = tag_level;


        if(unlikely(tag_level == 0)) {
            PRINT
            printk("%s: Tag %d with level %d is not existing.\n", MODNAME, tag, level);
            
            up_write(&(tag_entry -> level_lock[level]));
            if(atomic_dec_and_test(&(tag_entry -> waiting))) 
                tag_entry -> ready = 0;
            up_read(temp_sem);
            up_read(&(tag_lock[tag]));            
            return -EINVAL;
        }

        // This condition is necessary for the receiving thread to understand if the new level that has been created is already on "Ready state" (probably not)
        // In case it's in ready state, it should create a newer epoch level and receive on it, but in case ready == 0 it means that the new level is free and can be accessed
        if(tag_level -> ready == 1) {
            
            tag_level_t *new_tag_level; 
            new_tag_level = create_level(level, tag_level -> epoch + 1);
            if(unlikely(new_tag_level == 0)) {
                PRINT
                printk("%s: Could not create new level for Tag %d at level %d (epoch: %d -> %d)\n", 
                        MODNAME, tag, level, tag_level -> epoch, tag_level -> epoch + 1);
                
                up_write(&(tag_entry -> level_lock[level]));
                if(atomic_dec_and_test(&(tag_entry -> waiting))) 
                    tag_entry -> ready = 0;
                up_read(temp_sem);
                up_read(&(tag_lock[tag]));           
                return -ENOMEM;
            }


            //Overwrite the corresponding entry with the new level address
            tag_entry -> tag_level[level] = new_tag_level;
            asm volatile ("mfence" ::: "memory");

            tag_level = new_tag_level;
        }

        up_read(temp_sem);

        //Instantly take the new tag level RCU lock (Since the thread has moved to a next version of the level)
        if(unlikely(down_read_interruptible(&(tag_level -> rcu_lock)) == -EINTR)) {                
            PRINT
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            
            tag_entry -> tag_level[level] = old_level;

            up_write(&(tag_entry -> level_lock[level]));
            if(atomic_dec_and_test(&(tag_entry -> waiting))) 
                tag_entry -> ready = 0;
            up_read(&(tag_lock[tag]));
            
            return -EINTR;
        }

        up_write(&(tag_entry -> level_lock[level]));

        
        
    }
    
   

    // Even if in this frame a new level gets allocated (in order arrives x receiver, a sender and a reciver which will create the new level)
    // there's no problem even if no sender will evere be able to awake this receive thread(not counting CTL):
    // when the thread does "wait_event", it will trigger a "might_sleep()", which will check the ready condition and will
    // immediately wake it up
    
    atomic_inc(&(tag_level -> waiting));
    

    return_code = wait_event_interruptible(tag_level -> local_wq, tag_level -> ready || tag_entry -> ready);
    
    PRINT
    print_level(tag_level, tag);

    // When return_code == 0 it means it has been woken up, otherwise it was an interrupt
    if(return_code == 0) {
        if(tag_entry -> ready) return_code = 0;
        else if(tag_level -> ready) return_code = 1;
    }
    else return_code = 0;

    // If the return code is 1 it means it has been woken up by a "wake_up" call, and if tag_level -> ready == 1 it means there's
    // something to read in the buffer. Otherwise, the next steps are just skipped
    if(return_code == 1 && tag_level -> ready) {
        int current_size;
        current_size = min(size, tag_level -> size);
        // If current_size is 0, it won't copy anything, it will just wake up and go on
        if(current_size > 0 && buffer != 0)
            if(unlikely(copy_to_user(buffer, tag_level -> buffer, current_size)) != 0) {
                PRINT
                printk("%s: Could not copy the message to the User.\n", MODNAME);
                return_code = -EFAULT; 
            }
    }

    up_read(&(tag_level -> rcu_lock));


    //If the thread is the last one reading from the level
    if(atomic_dec_and_test(&(tag_level -> waiting))) {

        while(!down_write_trylock(&(tag_level -> rcu_lock))) {
            schedule();
        } 
        

        tag_level_t* new_tag_level;

        new_tag_level = (tag_entry -> tag_level)[level];
        if(unlikely(new_tag_level == 0)){
            PRINT
            printk("%s: Fatal Error: Could not access new level for tag %d at level %d \n", MODNAME, tag, level);
            
            if(atomic_dec_and_test(&(tag_entry -> waiting))) 
                tag_entry -> ready = 0;
            up_write(&(tag_level -> rcu_lock));
            up_read(&(tag_lock[tag]));
            return -EPROTO;
        }


        // If a next epoch exists it's possible to free the level...
        if(new_tag_level -> epoch > tag_level -> epoch) {
            PRINT {
                printk("%s: Deleting Tag %d Level %d of epoch %d\n", MODNAME, tag, level, tag_level -> epoch);
                //print_level(tag_level, tag);
            }
            up_write(&(tag_level -> rcu_lock));
            free_level(tag_level);
            
        
        } else { 
            // ... otherwise, the level gets re-initialized, 0-ing the values for the next send/receive
            PRINT {
                printk("%s: Clearing Tag %d Level %d of epoch %d\n", MODNAME, tag, level, tag_level -> epoch);
                //print_level(tag_level, tag);
            }
            //If a new tag level epoch doesn't exists set level_ready to 0 (the level is not used anymore)
            tag_level -> ready = 0;
            
            // Those two operation are not necessary since the next send will overwritre those value 
            // (beside the buffer which should be set to 0 for consistency purposes)
            // Should be removed to reduce the critical section. 
            tag_level -> size = 0;
            memset(tag_level -> buffer, 0, tag_level -> size);
            asm volatile ("sfence" ::: "memory");

            up_write(&(tag_level -> rcu_lock));
        }

        

       
    }
   
    if(atomic_dec_and_test(&(tag_entry -> waiting))) 
        tag_entry -> ready = 0;
    
    up_read(&(tag_lock[tag]));

    PRINT
    printk("%s: TAG_RECEIVE done. TID: %d, tag %d, level %d, buffer: %s, size: %ld\n", MODNAME, current->pid, tag, level, buffer, size);

    return return_code;
}









/**
 *  @brief  Signal a Tag
 *  
 *  @param  tag Tag descriptor of the Tag
 *  @param  command to signal wether is a Delete or Awake All command
 * 
 *  @return 1 on success, 0 on discarded command (no receiver waiting or tag occupied), negative error codes otherwise
 */
int tag_ctl(int tag, int command) {

    PRINT {
        char* command_str;
        if(command == TAG_AWAKE_ALL)    command_str = "TAG_AWAKE_ALL";
        else if(command == TAG_DELETE)  command_str = "TAG_DELETE";
        else                            command_str = "UNDEFINED";
        printk("%s: TAG_CTL called. TID: %d, tag: %d, command %s\n", MODNAME, current -> pid, tag, command_str);
    }
 
    // Input check
    if(tag < 0 || tag >= MAX_TAGS){
        PRINT
        printk("%s: TAG_CTL Wrong parameter usage\n", MODNAME);
        return -EINVAL;
    }

    if(command == TAG_AWAKE_ALL) {

        // Acquire Tag Lock
        if(unlikely(down_read_interruptible(&(tag_lock[tag])) == -EINTR)) {
            PRINT                
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            return -EINTR;
        }

        tag_t* tag_entry; 
        tag_entry = tags[tag];

        if(tag_entry == 0) {
            PRINT
            printk("%s: CTL with tag descriptor: %d is not existing.\n", MODNAME, tag);
            up_read(&(tag_lock[tag]));
            return -ENODATA;
        }

        // Root can always access
        if(CHECKPERM(tag_entry)) {
            PRINT
            printk("%s: Could not access the Tag service %d: permission error\n", MODNAME, tag);
            up_read(&(tag_lock[tag]));
            return -EPERM;
        }

        // Those next two if are separted to print distinguished info for the two cases
        if((tag_entry -> ready) == 1) {
            PRINT
            printk("%s: Tag %d is already making an Awake All.\b", MODNAME, tag);
            up_read(&(tag_lock[tag]));
            return 0;
        }

        if(atomic_read(&(tag_entry -> waiting)) == 0) {
            PRINT
            printk("%s: CTL AWAKE_ALL was called on tag %d but no receiver found\n", MODNAME, tag);
            up_read(&(tag_lock[tag]));
            return 0;
        }

        
        tag_entry -> ready = 1;

        tag_level_t* tag_level;
        int i;
        
        for(i = 0; i < LEVELS; i++) {

            if(unlikely(down_read_interruptible(&(tag_entry -> level_lock[i])) == -EINTR)) {                
                PRINT
                printk("%s: RW Lock was interrupted.\n", MODNAME);
                up_read(&(tag_lock[tag]));
                return -EINTR;
            }
           

            tag_level = (tag_entry -> tag_level)[i];

            if(likely(tag_level != 0)) {
                
                if(unlikely(down_read_interruptible(&(tag_level -> rcu_lock)) == -EINTR)) {                
                    PRINT
                    printk("%s: RW Lock was interrupted.\n", MODNAME);
                    up_read(&(tag_entry -> level_lock[i]));
                    up_read(&(tag_lock[tag]));
                    return -EINTR;
                }

                if(atomic_read(&(tag_level -> waiting)) > 0)    
                    wake_up_all(&(tag_level -> local_wq));

                up_read(&(tag_level -> rcu_lock)); 
            }

            up_read(&(tag_entry -> level_lock[i]));
                
        }

        up_read(&(tag_lock[tag]));


        PRINT
        printk("%s: CTL AWAKE_ALL done succesfully on tag %d\n", MODNAME, tag);


        return 1;


    } else if(command == TAG_DELETE) {
        
       
        // Even if the common data structures get (for the moment) only accessed in read, the write lock
        // is necessary to ensure single access in concurrent enviroment so it's not possible to have 2 thread
        // trying to delete the same tag
        /*
        if(unlikely(down_write_killable(&(tag_lock[tag])) == -EINTR)) {                
            PRINT
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            return -EINTR;
        }
        */

        // Since TAG_CTL is not requested to be a blocking service, it has been used a trylock which will succed only
        // if no one is using the specific tag
        // For the blocking variant just comment the trylock and uncomment the down_write_killable above
        if(!down_write_trylock(&(tag_lock[tag]))) {
           PRINT
           printk("%s: Could not delete tag %d, occupied\n", MODNAME, tag);
           return 0;
        }

        // Taking the common_lock in write will assure that no thread is accessing the common data structures
        // (i.e. no thread is trying to add/get/delete any tag and no thread is starting a tag_send or tag_rcv)
        tag_t* tag_entry; 
        tag_entry = tags[tag];

        if(tag_entry == 0) {
            PRINT
            printk("%s: CTL with tag descriptor: %d is not existing.\n", MODNAME, tag);
            up_write(&(tag_lock[tag]));
            return -ENODATA;
        }

        // Root can always access
        if(CHECKPERM(tag_entry)) {
            PRINT
            printk("%s: Could not access the Tag service %d: permission error\n", MODNAME, tag);
            up_write(&(tag_lock[tag]));
            return -EPERM;
        }

        // With this instruction the tag becomes unaccesible for other thread beside the one that are still making
        // a transaction
        // tags[tag] = 0 remove references to the tag, so no other thread can start a new operation on that tag
        tags[tag] = 0;

        up_write(&(tag_lock[tag]));

        

        // Check if someone is receiving (should be a useless check since we locked before, but still let's be sure)
        if(atomic_read(&(tag_entry -> waiting)) != 0) { 
            PRINT
            printk("%s: Critical Error! CTL DELETE was called on tag %d but still pending operation are present.\n", MODNAME, tag);
            tags[tag] = tag_entry;
            return -EPROTO;
        } 

        //From this point, no one is referncing this tag, so it can be safely deleted
        
       

        // clear_tag_common() is done here instead than above where tags[tag] is set to 0 to be able to restore the situation in case
        // any of the above instruction fails
        if(unlikely(clear_tag_common(tag_entry -> key, tag_entry -> tag_key) != 0)) {
            PRINT
            printk("%s: Fatal Error! Could not deallocate BM and HM for Tag %d.\n", MODNAME, tag_entry -> tag_key);
            tags[tag] = tag_entry;
            return -EINTR;
        }

        // Delete all levels
        clear_tag_level(tag_entry -> tag_level);

        if(tag_entry -> tag_level != 0) kfree(tag_entry -> tag_level);

        if(tag_entry != 0) kfree(tag_entry);

        PRINT
        printk("%s: CTL DELETE removed succesfully tag %d\n", MODNAME, tag);


        PRINT
        print_tag();

        return 1;
    }

    PRINT
    printk("%s: Command is invalid\n", MODNAME);
    return -EINVAL; 

}



// ---------------- Some helper function ---------------- \\



/**
 *  @brief  Allocate "LEVELS" levels and make tag_level reference them as
 *          a list of pointer to their memory position
 *  
 *  @param  tag_level variable used for storing the pointer to the new allocated levels
 *  
 *  @return -ENOMEM for failure in memory allocations, 0 for success 
 */
static int add_tag_level(tag_level_t** tag_level) {

    tag_level_t* level;
    int i;

    for(i = 0; i < LEVELS; i++) {
    
        level = create_level(i, 0);
        if(unlikely(level == 0)) return -ENOMEM;
        
        tag_level[i] = level;        
    }
    
    return 0;
}

/**
 *  @brief  Allocate and return a new Tag Level at a specific level on a specif epoch
 *  
 *  @param  i as the level of the new Tag Level
 *  @param  epoch of the Tag Level
 *        
 *  @return -ENOMEM for failure in memory allocations, 0 for success 
 */
static tag_level_t* create_level(int i, int epoch) {

    tag_level_t* level;
    char* buffer;

    level = kzalloc(sizeof(tag_level_t), GFP_KERNEL);
    if(unlikely(level == 0)) {
        PRINT
        printk("%s: Could not allocate memory level %d.\n", MODNAME, i);
        return 0;
    } 

    buffer = kzalloc(sizeof(char) * BUFFER_SIZE, GFP_KERNEL);
    if(unlikely(buffer == 0)) {
        PRINT
        printk("%s: Could not allocate buffer for level %d\n", MODNAME, i);
        kfree(level);
        return 0;
    }
    
    level -> level  = i;
    level -> buffer = buffer;
    level -> size   = 0;
    level -> ready  = 0;
    level -> epoch  = epoch;
    atomic_set(&(level -> waiting), 0);
    init_waitqueue_head(&(level -> local_wq));
    init_rwsem(&(level -> rcu_lock));
    mutex_init(&(level -> w_mutex));

    return level;

}


/**
 *  @brief  Clear common data structure (Hashmap and Bitmask) used for the specific tag
 *  
 *  @param  key associated to Tag descriptor tag_key
 *  @param  tag_key Tag Descriptor
 *  
 *  @return 0 on success, -EINTR if the lock was Killed
 */ 
static int clear_tag_common(int key, int tag_key) {

    // Lock the common data structure because it's necessary for write access
    if(unlikely(down_write_killable(&common_lock) == -EINTR)) {
        PRINT
        printk("%s: RW Lock was interrupted.\n", MODNAME);
        return -EINTR;
    }
    
    // If the key is IPC_PRIVATE, there will not be an entry in the hashmap to free
    if(key != IPC_PRIVATE) { 
        tag_table_entry_t* entry;
        entry = hashmap_get(tag_table, &(tag_table_entry_t){ .key = key});
        
        //Extra check which should not be necessary, but just to be safe
        if(likely(entry != 0 && entry -> tag_key == tag_key)) 
            hashmap_delete(tag_table, &(tag_table_entry_t){ .key = key});
        else
            PRINT
            printk("%s: Consistency error! Hashmap entry (key = %d, tag_desc = %d) and Tag entry (key = %d, tag_desc = %d) don't match.\n", 
                MODNAME, entry -> key, entry -> tag_key, key, tag_key);
    }
        
    if(unlikely(clear_number(tag_bitmask, tag_key) != 1)) {
        PRINT
        printk("%s: Critical error! Could not clear %d from bitmask.\n", MODNAME, tag_key);
        if(key != IPC_PRIVATE) hashmap_set(tag_table, &(tag_table_entry_t){ .key = key, .tag_key = tag_key});
    }

    // Release the lock 
    up_write(&common_lock); 

    return 0;   
}
 

/**
 *  @brief  Clear all the levels stored in tag_level
 *  
 *  @param  tag_level pointer to the array of the single levels pointers
 *  
 */ 
void clear_tag_level(tag_level_t** tag_level) {

    int i;
    for(i = 0; i < LEVELS; i++) 
        if(tag_level[i] != 0) 
            free_level(tag_level[i]);

}

/**
 *  @brief  Free a single level
 *  
 *  @param  tag_level pointer to the single level struct
 *  
 */ 
__always_inline static void free_level(tag_level_t* tag_level) {
    kfree(tag_level -> buffer);
    kfree(tag_level);
}


/**
 *  @brief  Print the whole content of all the Tags (the one that have been created) and the hashamp
 *          containing the mapping key -> tag descriptor
 *          Note: this function is used only for debug puposes, 
 *          so no locking mechanism optimization has been developed
 *  
 */ 
static void print_tag(void){

    int i;
    tag_t*  tag_ptr;
    tag_t   tag;
    tag_table_entry_t* tag_table_entry;

    printk("%s: Printing all Tags info\n", "PRINT-TAG");
   
    for(i = 0; i < MAX_TAGS; i++) {

        if(unlikely(down_read_interruptible(&(tag_lock[i])) == -EINTR)) {                
            PRINT
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            continue;
        }
                 
        tag_ptr = tags[i];
        
        if(tag_ptr != 0) {
           
            tag = *tag_ptr;

            printk("Key %d; Tag desc %d; Perm %d; Ready %d; Counter %d\n", 
                    tag.key, tag.tag_key, tag.permission, tag.ready, tag.waiting.counter);


        }

        up_read(&(tag_lock[i]));

    }

    if(unlikely(down_read_interruptible(&common_lock) == -EINTR)) {                
        PRINT
        printk("%s: RW Lock was interrupted.\n", MODNAME); 
        return;
    }


    printk("%s: Hahsmap content: %ld items\n", "PRINT-HASH", hashmap_count(tag_table));    
    

    for(i = 0; i < MAX_TAGS; i++) {


        if(unlikely(down_read_interruptible(&(tag_lock[i])) == -EINTR)) {                
            PRINT
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            up_read(&common_lock);
            continue;
        }
        
        tag_ptr = tags[i];
        
        if(tag_ptr != 0) {

            tag = *tag_ptr;
    
            tag_table_entry = hashmap_get(tag_table, &(tag_table_entry_t){ .key = tag.key});
            if(tag_table_entry != 0)
                printk("Key %d; Tag Key %d\n", 
                    tag_table_entry -> key, tag_table_entry -> tag_key);

            

        }

        up_read(&(tag_lock[i]));
 
    }
    
    up_read(&common_lock);
}


/**
 *  @brief  Print the content of the level specified (the one that have been created)
 *          Note: no lock has been introduced because the function gets called while 
 *          already having the level locked
 *  
 */ 
static void print_level(tag_level_t* tag_level, int tag) {
    
    if(tag_level == 0) return;
    printk("%s: (TID: %d) Tag: %d, level: %d, epoch: %d, waiting: %d (ready %d), size: %ld, buffer: %s \n", 
        "PRINT-LEVEL", current -> pid, tag, tag_level -> level, tag_level -> epoch, atomic_read(&(tag_level -> waiting)), tag_level -> ready, 
        tag_level -> size, tag_level -> buffer);

}


















// Syscall define and install syscall routines


__SYSCALL_DEFINEx(3, _tag_get, int, key, int, command, int, permission){ 
        int ret_val;
        if(!try_module_get(THIS_MODULE)) {
            printk("%s: Fatal Error: could not lock module!", MODNAME);
            return -1;
        }
        ret_val = tag_get(key, command, permission);
        module_put(THIS_MODULE);
        return ret_val;
}

unsigned long sys_tag_get;


__SYSCALL_DEFINEx(4, _tag_send, int, tag, int, level, char*, buffer, size_t, size) {
        int ret_val;
        if(!try_module_get(THIS_MODULE)) {
            printk("%s: Fatal Error: could not lock module!", MODNAME);
            return -1;
        }
        ret_val = tag_send(tag, level, buffer, size);
        module_put(THIS_MODULE);
        return ret_val;
}

unsigned long sys_tag_send;    


__SYSCALL_DEFINEx(4, _tag_receive, int, tag, int, level, char*, buffer, size_t, size) {
        int ret_val;
        if(!try_module_get(THIS_MODULE)) {
            printk("%s: Fatal Error: could not lock module!", MODNAME);
            return -1;
        }
        ret_val = tag_receive(tag, level, buffer, size);
        module_put(THIS_MODULE);
        return ret_val;
}

unsigned long sys_tag_receive;    


__SYSCALL_DEFINEx(2, _tag_ctl, int, tag, int, command) {
        int ret_val;
        if(!try_module_get(THIS_MODULE)) {
            printk("%s: Fatal Error: could not lock module!", MODNAME);
            return -1;
        }
        ret_val = tag_ctl(tag, command);
        module_put(THIS_MODULE);
        return ret_val;
}

unsigned long sys_tag_ctl;    


int install_syscalls(void) {
    
    sys_tag_get     = (unsigned long) __x64_sys_tag_get;
    sys_tag_send    = (unsigned long) __x64_sys_tag_send;
    sys_tag_receive = (unsigned long) __x64_sys_tag_receive;
    sys_tag_ctl     = (unsigned long) __x64_sys_tag_ctl;

    

    
    tag_get_nr      = syscall_insert((unsigned long *) sys_tag_get);
    tag_send_nr     = syscall_insert((unsigned long *) sys_tag_send);
    tag_receive_nr  = syscall_insert((unsigned long *) sys_tag_receive);
    tag_ctl_nr      = syscall_insert((unsigned long *) sys_tag_ctl);

    return tag_get_nr * tag_send_nr * tag_receive_nr * tag_ctl_nr;

}
