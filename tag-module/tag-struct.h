#include <linux/rwsem.h>

#define SEED0 401861
#define SEED1 879023
#define HASHMAP_CAP 256

#define BUFFER_SIZE 4096
#define LEVELS      32
#define MAX_TAGS    256

#define TAG_OPEN    0
#define TAG_CREAT   1

#define TAG_AWAKE_ALL   0
#define TAG_DELETE      1

// Error code used to comunicate that the max number of tag services has been reached
#define EMAXTAG     132



typedef struct tag_table_entry_struct {
    int key;
    int tag_key;
} tag_table_entry_t;

typedef struct tag_level_struct {
    int level;
    char* buffer;
    size_t size;
    int ready;
    int epoch;
    struct rw_semaphore rcu_lock;
    atomic_t waiting;
    wait_queue_head_t local_wq;
    
} tag_level_t;

typedef struct tag_struct {
    int key;
    int tag_key;
    int permission;
    int ready;
    struct rw_semaphore level_lock[LEVELS]; // Maybe this one can be converted to a plain mutex instead of a RW Semaphore
    atomic_t waiting;
    tag_level_t** tag_level;
} tag_t;



#ifndef TAG_STRUCT_H
#define TAG_STRUCT_H



#endif
