#include "bitmask.h"

static inline int get_free_bit(unsigned long long mask, int bits_per_slot);
static __always_inline void *alloc(size_t size);
static __always_inline void dealloc(void* obj);



bitmask* initialize_bitmask(int number_bits) {

    if(number_bits < 1) return 0;

    bitmask* bitmask;
    int slot_size = sizeof(unsigned long long);
    int number_slot = number_bits / (slot_size * 8) + (number_bits % (slot_size * 8) != 0);
    unsigned long long* mask = alloc(sizeof(unsigned long long) * number_slot);
    if(mask == 0) return 0;

    bitmask = alloc(sizeof(bitmask));
    if(bitmask == 0) return 0;

    bitmask->mask           = mask;
    bitmask->n_bits         = number_bits;
    bitmask->slots          = number_slot;
    bitmask->slot_size      = slot_size;
    bitmask->bits_per_slot  = slot_size * 8;
}

void free_bitmask(bitmask* bitmask) {

    dealloc(bitmask -> mask);
    dealloc(bitmask);
}

int get_avail_number(bitmask* bitmask) {
     
    for(int i = 0; i < bitmask -> slots; i++) {
        
        unsigned long long slot_mask = *((bitmask -> mask) + i);
        
        int free_bit = get_free_bit(slot_mask, bitmask -> bits_per_slot);
      

        if(free_bit != -1){
            int number = free_bit + i * (bitmask -> bits_per_slot);
            
            if(number >= bitmask -> n_bits) return -1;
            SET_BIT(((bitmask -> mask) + i), free_bit);
            return number;
        }  
        
    }

    return -1;

}


int clear_number(bitmask* bitmask, int number){

    // Get correct mask slot
    int slot = number / (sizeof(unsigned long long) * 8);
    
    if(number >= bitmask -> n_bits) return -1;

    int bit = number % bitmask -> bits_per_slot; 


    unsigned long long slot_mask = *((bitmask -> mask) + slot);
    
    if(CHECK_BIT(slot_mask, bit) == 0) return 0;

    CLEAR_BIT(((bitmask -> mask) + slot), bit);

    return 1;

}


static inline int get_free_bit(unsigned long long mask, int bits_per_slot) {
    for(int bit = 0; bit < bits_per_slot; bit++) {
        if(CHECK_BIT(mask, bit) == 0) return bit;
    }
    return -1;
}

#ifdef TEST_FUNC
static __always_inline void *alloc(size_t size) { return calloc(size / sizeof(unsigned long long), sizeof(unsigned long long)); }
#else
static __always_inline void *alloc(size_t size) { return kzalloc(size, GFP_ATOMIC); }
#endif

#ifdef TEST_FUNC
static __always_inline void dealloc(void* obj) { free(obj); }
#else
static __always_inline void dealloc(void* obj) { kfree(obj); }
#endif