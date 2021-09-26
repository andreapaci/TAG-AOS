#include "module.h"
#include "ptexplorer.h"


// Start and End address for Kernel Address Space and 
#define START_ADDR		    ((unsigned long long) 0xffffffff00000000ULL)
#define END_ADDR		    ((unsigned long long) 0xfffffffffff00000ULL)

// Number of syscall entries in the table to analize
#define TABLE_ENTRIES       ((int) 256)

// Free entries in the syscall table (pointing at sys_ni_syscall)
#define FREE_ENTRIES        ((int) 7)
// Mettere ni_syscall**************************************

// Logical page size in bytes
#ifndef PAGE_SIZE_DEF
#define PAGE_SIZE_DEF ((unsigned long long) 4096ULL)
#endif

//Mask to get the page number (same as doing >> 12)
#define PAGE_BITMASK ((unsigned long long) 0xfffffffffffff000ULL)
  

// scth_disc.c
void find_syscall_table(void);
int syscall_insert(unsigned long* syscall_function);
void syscall_clean(void);
