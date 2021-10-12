#define SEED0 401861
#define SEED1 879023
#define HASHMAP_CAP 256

#define BUFFER_SIZE 4096
#define LEVELS      32
#define MAX_TAGS    256

#define TAG_OPEN    0
#define TAG_CREAT   1

// Error code used to comunicate that the max number of tag services has been reached
#define EMAXTAG     132



typedef struct tag_table_entry_struct {
    int key;
    int tag_key;
} tag_table_entry;


#ifndef TAG_STRUCT_H
#define TAG_STRUCT_H

int tag_compare(const void* a, const void* b, void* udata) {
    return ((tag_table_entry *) a) -> key - ((tag_table_entry *) b) -> key;
}

uint64_t tag_hash(const void *item, uint64_t seed0, uint64_t seed1 ) {
    const tag_table_entry* entry = item;
    return hashmap_sip( &(entry -> key), sizeof(int), seed0, seed1);
}

#endif
