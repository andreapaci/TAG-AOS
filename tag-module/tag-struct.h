#define SEED0 401861
#define SEED1 879023
#define HASHMAP_CAP 1024

#define BUFFER_SIZE 4096
#define LEVELS      32
#define MAX_TAGS    256

#define TAG_OPEN    0
#define TAG_CREAT   1



typedef struct tag_table_entry_struct {
    int key;
    int tag_key;
    char* buffer;
} tag_table_entry;

int tag_compare(const void* a, const void* b, void* udata) {
    return ((tag_table_entry *) a)->tag_key - ((tag_table_entry *)b)->tag_key;
}