#ifdef TEST_FUNC
#include <malloc.h>
#include <stdio.h>
#else
#include <linux/slab.h>
#endif

#define CHECK_BIT(mask, pos)    ((mask) &   (1ULL << pos))
#define SET_BIT(mask, pos)     ((*mask) |=  (1ULL << pos))
#define CLEAR_BIT(mask, pos)   ((*mask) &= ~(1ULL << pos))

typedef struct bitmask_struct {
    unsigned long long*  mask;
    int n_bits;
    int slots;
    int slot_size;
    int bits_per_slot;
    

} bitmask;

bitmask* initialize_bitmask(int number_bits);
void clear_bitmask(bitmask* bitmask);
int get_avail_number(bitmask* bitmask);
int clear_number(bitmask* bitmask, int number);