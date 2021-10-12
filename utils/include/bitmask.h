typedef struct bitmask_struct {
    unsigned long long*  mask;
    int n_bits;
    int slots;
    int slot_size;
    int bits_per_slot;
    

} bitmask_t;

bitmask_t* initialize_bitmask(int number_bits);
void free_bitmask(bitmask_t* bitmask);
int get_avail_number(bitmask_t* bitmask);
int clear_number(bitmask_t* bitmask, int number);