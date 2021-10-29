/**
 *  @file   tag_struct.c
 *  @brief  Header file with the definition of some of the struct used in the module
 *  @author Andrea Paci
 */ 


#include <linux/rwsem.h>
#include "include/tag.h"

#define SEED0 401861
#define SEED1 879023
#define HASHMAP_CAP MAX_TAGS * 2

#define BUFFER_SIZE 4096
#define LEVELS      32
#define MAX_TAGS    256

#define CHECKPERM(tag_entry) (tag_entry -> permission == TAG_PERM_USR && current_euid().val != 0 && tag_entry -> euid != current_euid().val)

// Single entry og the Hashmap
typedef struct tag_table_entry_struct {
    int key;
    int tag_key;
} tag_table_entry_t;

// Struct used to describe a single level of a Tag Service
typedef struct tag_level_struct {
    int level;              // Level of the Tag Level                  
    size_t size;            // Size of the message
    int ready;              // Signal wether the tag level is occupied in a Tag Send (1) or not (0)
    int epoch;              // Level Epoch (RCU alike)
    atomic_t waiting;       // Number of waiting receiving thread on this level
    wait_queue_head_t       /* Wait Queue for receiving thread waiting for the message delivery */
            local_wq;
    struct rw_semaphore     /* RW Semaphore to synsconize the access to the single level instance*/
            rcu_lock;
    struct mutex w_mutex;   // Mutex used to block concurrent send   
    // Buffer for message exchange
    char __attribute__((aligned(PAGE_SIZE))) *buffer;                   
    
} tag_level_t;

// Struct used to describe a single Tag Service entry
typedef struct tag_struct {
    int key;                    // Key used to create the Tag               
    int tag_key;                // Tag descriptor
    int ready;                  // Signal wether the tag is occupied in a AWAKE_ALL (1) or not (0)
    int permission;             // Indicates if the Tag can be accessed by all user or only by the user who created the tag
    uid_t euid;                 // Effective User ID related to the task calling the system call
    tag_level_t** tag_level;    // List of pointers to the various levels
    atomic_t waiting;           // Number of Receiving thread on this Tag
    struct rw_semaphore         /* RW Semaphore to syncronize access to the pointer list of levels */
        level_lock[LEVELS]; 
} tag_t;