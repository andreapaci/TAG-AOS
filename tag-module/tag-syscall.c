/**
 *  @file   tag-syscall.c
 *  @brief  Source code for the definition of the system calls relative to TAG module
 *  @author Andrea Paci
 */ 


#include "module.h"

static int  add_tag_level(tag_level_t** tag_level);
static tag_level_t* create_level(int i);
static void clear_tag(tag_t* tag_entry);
static void clear_tag_level(tag_level_t** tag_level);
__always_inline static void free_level(tag_level_t* tag_level);
static void print_tag(void);


int tag_get(int key, int command, int permission) {

    PRINT {
        
        char* command_str;
        if(command == TAG_OPEN) command_str = "TAG_OPEN";
        else if(command == TAG_CREAT) command_str = "TAG_CREAT";
        else command_str = "UNDEFINED";

        printk("%s: TAG_GET has been called with key %d and command %s\n", MODNAME, key, command_str);

    }
    
    // Impostare un try_lock che se fallisce (con stessa chiave) allora vuol dire che qualcun altro sta già creando lo stesso servizio (che si fa?)
    //Serializza l'operazioni su dati comuni come la bitmask
    if(key < 0) {
        PRINT
        printk("%s: Key is invalid (< 0)\n", MODNAME);
        return -EINVAL; 
    }

    // Creating new Tag Service
    if(command == TAG_CREAT) {
        
        if(unlikely(down_write_killable(&common_lock) == -EINTR)) {
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            return -EINTR;
        }
        
        // Check if more Tag services can be created 
        if(hashmap_count(tag_table) >= MAX_TAGS) {
            PRINT
            printk("%s: Maximum Tag services reached (%s)\n", MODNAME, MAX_TAGS);
            
            up_write(&common_lock);
            
            return -EMAXTAG;
        }

        // Get available tag number
        int tag_key; 
        
        tag_key = get_avail_number(tag_bitmask);
        if(tag_key < 0) {
            PRINT
            printk("%s: No tag_key avaliable\n", MODNAME);
            up_write(&common_lock);
            return -EPROTO;
        }

        //if key IPC_PRIVATE no need to add it to the tag_table hashmap 
        // (it will never be necessary to get key - tag descriptor mapping)
        if(key != IPC_PRIVATE) {
            
            // Check if a Tag with the same key is already existing
            if(hashmap_get(tag_table, &(tag_table_entry_t){ .key = key}) != 0) {
                PRINT
                printk("%s: Tag with key %d already existing.\n", MODNAME, key);
                
                //c9lear_tag(tag_key, 0, 0);
                if(clear_number(tag_bitmask, tag_key) != 1) 
                    printk("%s: Could not clear %d from bitmask.\n", MODNAME, tag_key);

                up_write(&common_lock);
            
                return -EBUSY;
            }

            
            // Add new entry to the hashmap          
            if(unlikely(
                hashmap_set(tag_table, &(tag_table_entry_t){ .key = key, .tag_key = tag_key}) == 0 && 
                hashmap_oom(tag_table)
                )) {
                
                PRINT
                printk("%s: Could not allocate hash struct entry.\n", MODNAME);
                
                //c9lear_tag(tag_key, 0, 0);
                if(clear_number(tag_bitmask, tag_key) != 1) 
                    printk("%s: Could not clear %d from bitmask.\n", MODNAME, tag_key);

                up_write(&common_lock);
            
                return -ENOMEM;
            }

            
        }

        // No write on common struct needed for now, the subsequent code will only allocate struct used to represent
        // the new tag service (beside on a failure of these allocatation that requires freeing the common data structures)
        up_write(&common_lock);


        // Alloc TAG service buffer        
        tag_level_t** tag_level;
        tag_level = kzalloc(sizeof(tag_level_t*) * LEVELS, GFP_KERNEL);
        if(unlikely(tag_level == 0)) {
            PRINT
            printk("%s: Could not allocate memory for Tag Service levels array.\n", MODNAME);
            
            if(unlikely(down_write_killable(&common_lock) == -EINTR)) {
                printk("%s: RW Lock was interrupted.\n", MODNAME);
                return -EINTR;
            }
            
            // If the key is IPC_PRIVATE, there will not be an entry in the hashmap
            if(key != IPC_PRIVATE) {
                tag_table_entry_t* entry;
                entry = hashmap_get(tag_table, &(tag_table_entry_t){ .key = key});
                
                //Extra check which should not be necessary, but just to be safe
                if(entry != 0 && entry -> tag_key == tag_key)    
                    hashmap_delete(tag_table, &(tag_table_entry_t){ .key = key});
            }

            //c9lear_tag(tag_key, 0, 0);
            if(clear_number(tag_bitmask, tag_key) != 1) 
                printk("%s: Could not clear %d from bitmask.\n", MODNAME, tag_key);
            
            
            up_write(&common_lock);
            
            return -ENOMEM;
        }
        
        
        if(add_tag_level(tag_level) != 0) {
            
            PRINT
            printk("%s: Could not allocate memory for Tag Service single levels.\n", MODNAME);
            
            
            clear_tag_level(tag_level);

            //c9lear_tag(tag_key, tag_level, 0);
            kfree(tag_level);

            if(unlikely(down_write_killable(&common_lock) == -EINTR)) {
                printk("%s: RW Lock was interrupted.\n", MODNAME);
                return -EINTR;
            }

            
            // If the key is IPC_PRIVATE, there will not be an entry in the hashmap
            if(key != IPC_PRIVATE) {
                tag_table_entry_t* entry;
                entry = hashmap_get(tag_table, &(tag_table_entry_t){ .key = key});
                
                //Extra check which should not be necessary, but just to be safe
                if(entry != 0 && entry -> tag_key == tag_key)    
                    hashmap_delete(tag_table, &(tag_table_entry_t){ .key = key});
            }

            if(clear_number(tag_bitmask, tag_key) != 1) 
                printk("%s: Could not clear %d from bitmask.\n", MODNAME, tag_key);


            up_write(&common_lock);

            return -ENOMEM;
        }

        tag_t* tag_entry = kzalloc(sizeof(tag_t), GFP_ATOMIC);
        if(unlikely(tag_entry == 0)) {
            PRINT
            printk("%s: Could not allocate memory for Tag Service entry.\n", MODNAME);

            kfree(tag_entry);
            
            clear_tag_level(tag_level);

            //c9lear_tag(tag_key, tag_level, 0);
            kfree(tag_level);

            if(unlikely(down_write_killable(&common_lock) == -EINTR)) {
                printk("%s: RW Lock was interrupted.\n", MODNAME);
                return -EINTR;
            }

            
            // If the key is IPC_PRIVATE, there will not be an entry in the hashmap
            if(key != IPC_PRIVATE) {
                tag_table_entry_t* entry;
                entry = hashmap_get(tag_table, &(tag_table_entry_t){ .key = key});
                
                //Extra check which should not be necessary, but just to be safe
                if(entry != 0 && entry -> tag_key == tag_key)    
                    hashmap_delete(tag_table, &(tag_table_entry_t){ .key = key});
            }

            if(clear_number(tag_bitmask, tag_key) != 1) 
                printk("%s: Could not clear %d from bitmask.\n", MODNAME, tag_key);


            up_write(&common_lock);

            return -ENOMEM;
        }


        tag_entry -> key = key;
        tag_entry -> tag_key = tag_key;
        tag_entry -> permission = permission;
        tag_entry -> ready = 0;
        init_rwsem(&(tag_entry -> tag_lock));
        atomic_set(&(tag_entry -> waiting), 0);
        tag_entry -> tag_level = tag_level;
        
        
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

        if(tags[tag_key] == 0){
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


//NOTA--------------------------
//Inoltre, ricordati che quanti si fa tag_send e recv, fare un controllo preventivo se il buffer esiste
//inoltre, allo smontaggio, elimina tutte le entry
//Fai le FREE



// Questo sistema strano di lockare le cose (prima common e poi tag_lock) è fatto per evitare che che venga eliminato il tag mentre si
// sta cercando di accederci, risultando in errori di accesso in memoria illecito
int tag_send(int tag, int level, char* buffer, size_t size) { 


    PRINT
    printk("%s: Tag send has been called.\n", MODNAME);

    if(unlikely(down_read_interruptible(&common_lock) == -EINTR)) {                
        printk("%s: RW Lock was interrupted.\n", MODNAME);
        return -EINTR;
    }

    tag_t* tag_entry;
    tag_entry = tags[tag];

    if(tag_entry == 0) {
        PRINT
        printk("%s: Tag %d is not created.\b", MODNAME, tag);
        up_read(&common_lock);
        //Metti val ritrono corretto   
        return -1;
    }

    //FAI LA STESSA STRUTTURA DEL SEND ANCHE SU AWAKE_ALL
    if(atomic_read(&(tag_entry -> waiting)) == 0) {
        PRINT
        printk("%s: Tag %d has no reader.\b", MODNAME, tag);
        up_read(&common_lock);
        return -1;
    }

    if(unlikely(down_read_interruptible(&(tag_entry -> tag_lock)) == -EINTR)) {                
        printk("%s: RW Lock was interrupted.\n", MODNAME);
        up_read(&common_lock);
        return -EINTR;
    }
    up_read(&common_lock);

    tag_level_t* tag_level;
    tag_level = (tag_entry -> tag_level)[level];

    if(tag_level == 0) {
        PRINT
        printk("%s: Tag %d with level %d is not existing.\n", MODNAME, tag, level);
        up_read(&(tag_entry -> tag_lock));
        return -EINTR;
    }

    if(atomic_read(&(tag_level -> waiting)) == 0) {
        PRINT
        printk("%s: Tag %d on level %d has no reader.\b", MODNAME, tag, level);
        up_read(&(tag_entry -> tag_lock));
        return -1;
    }

    tag_level -> ready = 1;
    wake_up_all(&(tag_level -> local_wq));

    /*
    int schedule_calls;
    schedule_calls = 0;
    
    while(atomic_read(&(tag_level -> waiting)) != 0) { schedule(); schedule_calls++; }
    
    PRINT
    printk("%s: [Send] Number of schedule calls : %d\n", MODNAME, schedule_calls);

    */
    up_read(&(tag_entry -> tag_lock));

    printk("Send done: %d - %d\n", tag_level -> ready, atomic_read(&(tag_level -> waiting)));

    //Aspettare che tutto ritorna come prima e risettare il livello al valore corretto, tra cui ready = 0

    //CONTROLLI SUGLI INPUT (LEVEL >= 0, SIZE >= 0, ETC, SE IL TAG ESISTE)
    
    return 1;
}

int tag_receive(int tag, int level, char* buffer, size_t size) { 

    PRINT
    printk("%s: Tag receive has been called.\n", MODNAME);

    if(unlikely(down_read_interruptible(&common_lock) == -EINTR)) {                
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            return -EINTR;
    }

    tag_t* tag_entry;
    tag_entry = tags[tag];

    if(tag_entry == 0) {
        PRINT
        printk("%s: Tag %d is not created.\b", MODNAME, tag);
        up_read(&common_lock);
        //Metti val corretto   
        return -1;
    }

    

   /* // Se è attivo, spostati in un nuovo buffer e andiamo in un'altra epoca
    if(tag_entry -> ready == 1) {
        //if(add_tag_level(tag, level) != 0) {
            up_write(&(tag_entry -> tag_lock));
            up_read(&common_lock);
            return -ENOMEM;
       // }
    }*/
    
    // Fare check atomici se dobbiamo saltare di epoca oppure il livello non è ancora stato creato
    if(unlikely(down_read_interruptible(&(tag_entry -> tag_lock)) == -EINTR)) {                
        printk("%s: RW Lock was interrupted.\n", MODNAME);
        up_read(&common_lock);
        return -EINTR;
    }

    up_read(&common_lock);

    tag_level_t* tag_level;
    tag_level = (tag_entry -> tag_level)[level];

    if(tag_level == 0) {
        PRINT
        printk("%s: Tag %d with level %d is not existing.\n", MODNAME, tag, level);
        up_read(&(tag_entry -> tag_lock));
        return -EINTR;
    }
    

    atomic_inc(&(tag_entry -> waiting));
    atomic_inc(&(tag_level -> waiting));
    

    printk("Pre_sleep %d - %d\n", tag_level -> ready, atomic_read(&(tag_level -> waiting)));
    
    wait_event_interruptible(tag_level -> local_wq, tag_level -> ready || tag_entry -> ready );

    //CONTROLLI SUGLI INPUT (LEVEL >= 0, SIZE >= 0, ETC, CONTROLLARE SE IL TAG ESISTE)

    atomic_dec(&(tag_level -> waiting));
    atomic_dec(&(tag_entry -> waiting));

    if(atomic_read(&(tag_level -> waiting)) == 0) {
        // SE SONO L'ULTMO A LEGGERE, ALLORA LIBERO LA SITUAZINE E IMPOSTO tag_level -> ready a 0
    }

   printk("Post_sleep %d - %d\n", tag_level -> ready, atomic_read(&(tag_level -> waiting)));

   up_read(&(tag_entry -> tag_lock));


    return 2;
}

int tag_ctl(int tag, int command) {

    PRINT {
        
        char* command_str;
        if(command == TAG_AWAKE_ALL) command_str = "TAG_AWAKE_ALL";
        else if(command == TAG_DELETE) command_str = "TAG_DELETE";
        else command_str = "UNDEFINED";
        printk("%s: TAG_CTL has been called with tag key %d and command %s\n", MODNAME, tag, command_str);
    }
 

    if(command == TAG_AWAKE_ALL) {

        // VEDI SE QUESTO LOCK VA PRESO QUI
        if(unlikely(down_read_interruptible(&common_lock) == -EINTR)) {                
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            return -EINTR;
        }

        tag_t* tag_entry; 
        tag_entry = tags[tag];

        if(tag_entry == 0) {
            PRINT
            printk("%s: CTL with tag descriptor: %d is not existing.\n", MODNAME, tag);
            up_read(&common_lock);
            return -ENODATA;
        }

        if(atomic_read(&(tag_entry -> waiting)) == 0) {
            PRINT
            printk("%s: CTL AWAKE_ALL was called on tag %d but no receiver found\n", MODNAME, tag);
            up_read(&common_lock);
            return 12;
        }

        if(unlikely(down_read_interruptible(&(tag_entry -> tag_lock)) == -EINTR)) {                
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            up_read(&common_lock);    
            return -EINTR;
        }

        up_read(&common_lock);

        


        

        //NO ONE RESETS THE READY BIT TO 0
        // Cosa succede se per esempio su tutti i livelli viene fatta una send e tutti i livelli hanno un receiver?
        // Ogni reader dovrà scegliere di impostare un bit a 0, se quello di tag.ready o tag_level.ready
        // Soluzione:: se tag.waiting = 0, ready viene impostato a 0 a prescindere se ci si è svegliati per una send
        // op per un awake all (idea da pensare e testare)
        // FAI TEST_SET SU WAITING SIA DI LIVELLO CHE DI TAG, E QUANDO SONO 0, IMPOSTALI A 0 
        //  (CHIARAMENTE CONTROLLI SEPARATI, UNO PER LEVEL E UNO PER TAG)
        // INOLTRE, NON E' UN PROBLEMA DI CONCORRENZA, PERCHE CTL E SEND DEVONO VERIFICARE CHE CI STANNO DEI RICEVENTI PRIMA DI FARE READY = 1
        // QUINDI NON CI SARA MAI UNA SITUAZIONE DOVE VIENE IMPOSTATO A 0 MA E' PARTITO UN SEND/CTL CORRETTO
        tag_entry -> ready = 1;

        tag_level_t* tag_level;
        int i;
        
        for(i = 0; i < LEVELS; i++) {


            tag_level = tag_entry -> tag_level[i];
            if(tag_level != 0)
                if(atomic_read(&(tag_level -> waiting)) > 0) {
                    
                    wake_up_all(&(tag_level -> local_wq));
                }
        }

        up_read(&(tag_entry -> tag_lock));


    } else if(command == TAG_DELETE) {
        
        

        // VEDI SE QUESTO LOCK VA PRESO QUI
        // Even if the common data structures get (for the moment) only accessed in read, the write lock
        // is necessary to ensure single access in concurrent enviroment so it's not possible to have 2 thread
        // trying to delete the same tag
        if(unlikely(down_write_killable(&common_lock) == -EINTR)) {                
            printk("%s: RW Lock was interrupted.\n", MODNAME);
                
            return -EINTR;
        }

        //Taking the common_lock in write will assure that no thread is accessing the common data structures
        // (ie no thread is trying to add/get/delete any tag and no thread is starting a tag_send or tag_rcv)
        tag_t* tag_entry; 
        tag_entry = tags[tag];
        if(tag_entry == 0) {
            PRINT
            printk("%s: CTL with tag descriptor: %d is not existing.\n", MODNAME, tag);
            up_write(&common_lock);
            return -ENODATA;
        }

        // With this instruction the tag becomes unaccesible for other thread beside the one that are still making
        // a transaction
        // tags[tag] = 0 remove references to the tag, so no other thread can start a new operation on that tag
        tags[tag] = 0;

        up_write(&common_lock);

        // Wait for other threasd to finish the transaction on this tag
        // Probably this lock is unnecessary since every transaction is made by an escalation of lock acquire, so the acquire of
        // common_lock as writer should be enough, but still added for increased security
        if(unlikely(down_write_killable(&(tag_entry -> tag_lock)) == -EINTR)) {                
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            tags[tag] = tag_entry;
            return -EINTR;
        }

        //check if someone is receiving (add counter to both levels and global)
        if(atomic_read(&(tag_entry -> waiting)) != 0) {  //PROBABILMENTE SUPERFLUO
            PRINT
            printk("%s: CTL DELETE was called on tag %d but still pending operation are present.\n", MODNAME, tag);
            tags[tag] = tag_entry;
            up_write(&(tag_entry -> tag_lock));
            return 12;
        } 

        //From this point, no one is referncing this tag, so it can be safely deleted
        
        // We can now release the lock since no thread can reach the tag, so it's possible to safely delete
        up_write(&(tag_entry -> tag_lock));

        


        // Lock to modify common access structures
        if(unlikely(down_write_killable(&common_lock) == -EINTR)) {                
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            tags[tag] = tag_entry;

            return -EINTR;
        }

        // clear_tag() is done here instead than above where tags[tag] is set to 0 to be able to restore the situation in case
        // any of the above instruction fails (there's no way)
        clear_tag(tag_entry);

        up_write(&common_lock);

        // Delete all leveles
        clear_tag_level(tag_entry -> tag_level);

        if(tag_entry -> tag_level != 0) kfree(tag_entry -> tag_level);

        if(tag_entry != 0) kfree(tag_entry);


        printk("%s: CTL DELETE removed succesfully tag %d\n", MODNAME, tag);


        //unlock
        PRINT
        print_tag();

    }

    // INSERISCI WRONG COMMAND USAGE con return errorcode EINVAL o simili

    return 3;
}









// Utility and common functions








static int add_tag_level(tag_level_t** tag_level) {

    
    int i;
    for(i = 0; i < LEVELS; i++) {
    
   
        tag_level_t* level;
        level = create_level(i);

        if(level == 0) return -1;
        
        tag_level[i] = level;
        
    }

    
    return 0;
}

static tag_level_t* create_level(int i) {

    tag_level_t* level;
    level = kzalloc(sizeof(tag_level_t), GFP_KERNEL);
    if(level == 0) {
        PRINT
        printk("%s: Could not allocate memory level %d.\n", MODNAME, i);
        //Unlock
        return 0;
    } 

    char* buffer;
    buffer = kzalloc(sizeof(char) * BUFFER_SIZE, GFP_KERNEL);
    if(buffer == 0){
        PRINT
        kfree(level);
        printk("%s: Could not allocatore buffer for level %d\n", MODNAME, i);
        //Unlock
        return 0;
    }
    
    level -> level = i;
    level -> ready = 0;
    level -> buffer = buffer;
    atomic_set(&(level -> waiting), 0);
    init_waitqueue_head(&(level -> local_wq));
    init_rwsem(&(level -> level_lock));

    return level;

}


static void clear_tag(tag_t* tag_entry) {

    int key;
        
    key = tag_entry -> key;
    if(key != IPC_PRIVATE) { 
        
        tag_table_entry_t* entry;
        entry = hashmap_get(tag_table, &(tag_table_entry_t){ .key = key});
        
        if(entry != 0 && entry -> tag_key == tag_entry -> tag_key) { // Extra check just to be sure
            hashmap_delete(tag_table, &(tag_table_entry_t){ .key = key});
        } else {
            printk("%s: Consistency error! Hashmap entry (key = %d, tag_desc = %d) and Tag entry (key = %d, tag_desc = %d) doesn't match.\n", 
            MODNAME, entry -> key, entry -> tag_key, tag_entry -> key, tag_entry -> tag_key);
        }

    }
        
    if(clear_number(tag_bitmask, tag_entry -> tag_key) != 1)
        printk("%s: Could not clear %d from bitmask.\n", MODNAME, tag_entry -> tag_key);

}


static void clear_tag_level(tag_level_t** tag_level) {

    int i;
    for(i = 0; i < LEVELS; i++) 
        if(tag_level[i] != 0) 
            free_level(tag_level[i]);

}

__always_inline static void free_level(tag_level_t* tag_level) {
    kfree(tag_level -> buffer);
    kfree(tag_level);
}

static void print_tag(void){

    int i;
    tag_t*  tag_ptr;
    tag_t   tag;
    tag_table_entry_t* tag_table_entry;

    //NOTA RIVEDI BENE L'USO DI LOCK

    // This lock has to be done because in the frame between "if(tag_ptr != 0)" and the subsequent down_read(tag_ptr -> tag_lock)
    // the tag gets deleted, there will be a memory access to a freed memory
    // This lock can also be taken at each for iteration and released as soon as the tag_lock is taken, but
    // since this operation is done to snapshot the situation, possibly avoiding external add/removal of tags
    // while running, the lock has been set here. Moreover, it's a debugging functionality, not meant to be used in a stable
    // release, so no particular optimization has been thought 

    if(unlikely(down_read_interruptible(&common_lock) == -EINTR)) {                
        printk("%s: RW Lock was interrupted.\n", MODNAME);
                
        return;
    }

    for(i = 0; i < MAX_TAGS; i++) {

                 
        tag_ptr = tags[i];
        
        if(tag_ptr != 0) {
           
            // FORSE TAG_LOCK NON SERVE
            if(unlikely(down_read_interruptible(&(tag_ptr -> tag_lock)) == -EINTR)) {                
                printk("%s: RW Lock was interrupted.\n", MODNAME);
                continue;
            }

            //SE SI VUOLE AGGIUNGERE LA PRINT ANCHE DEI SINGOLI LIVELLI, È NECESSARIO LOCKARE I SINGOLI LIVELLI

            tag = *tag_ptr;

            printk("%s: Tag struct Key: %d; Tag Key: %d\n\t\tPermission: %d\n\t\tReady: %d; Counter: %d\n", MODNAME, 
                    tag.key, tag.tag_key, tag.permission, tag.ready, tag.waiting.counter);

            up_read(&(tag_ptr -> tag_lock));

        }

    }

   printk("%s: Hahsmap content: %ld items\n", MODNAME, hashmap_count(tag_table));    
    
   for(i = 0; i < MAX_TAGS; i++) {

        
        tag_ptr = tags[i];
        
        if(tag_ptr != 0) {

            if(unlikely(down_read_interruptible(&(tag_ptr -> tag_lock)) == -EINTR)) {
                printk("%s: RW Lock was interrupted.\n", MODNAME);
                continue;
            }
            
            tag = *tag_ptr;
    
            tag_table_entry = hashmap_get(tag_table, &(tag_table_entry_t){ .key = tag.key});
            printk("%s: Tag Table Entry: Key: %d; Tag Key: %d\n", MODNAME, 
                    tag_table_entry -> key, tag_table_entry -> tag_key);

            
            up_read(&(tag_ptr -> tag_lock));

        } 
    }
    
    up_read(&common_lock);
}


















// Syscall define and install syscall routines


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








