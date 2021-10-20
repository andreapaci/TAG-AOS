#include "../module.h"

#define PT_CR3_MASK 0xfffffffffffff000ULL   // Mask used to get only the bits relative to page table address in CR3

unsigned long long get_pt_addr(void); // __attribute__((always_inline));

void enable_WP(unsigned long* flags, unsigned long cr0); // __attribute__((always_inline));
void disable_WP(unsigned long* flags, unsigned long cr0); // __attribute__((always_inline));