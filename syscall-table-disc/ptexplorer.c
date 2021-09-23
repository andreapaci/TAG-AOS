/**
 *  @file   pte.c
 *  @brief  Source code for Page Table Explorer in a x86-64 system and checking
 *          wether a specific virtual address is mapped in physical memory 
 *  @author Andrea Paci
 */ 


#include "ptexplorer.h"


#define PML4(addr) (((long long)(addr) >> 39) & 0x1ff)
#define PDP(addr)  (((long long)(addr) >> 30) & 0x1ff)
#define PDE(addr)  (((long long)(addr) >> 21) & 0x1ff)
#define PTE(addr)  (((long long)(addr) >> 12) & 0x1ff)

#define PT_CR3_MASK     0xfffffffffffff000ULL   // Mask used to get only the bits relative to page table address in CR3
#define PRESENT_BIT     0x1ULL                  // Mask to check the Present bit
#define PT_ADDRESS_MASK 0x7ffffffffffff000      // Mask to get the subsequent page table address (or huge page)
#define H_PAGES         0x80ULL                 // Mask to check wether the PT entry points to another PT or a frame (1GB/2MB)                               




// Get Page Table address reading CR3 register
static inline unsigned long get_pt_addr(void) {
    unsigned long addr;
    asm volatile("mov %%cr3,%0":  "=r" (addr) : );
    return addr & PT_CR3_MASK;
}


/**
 *  @brief  Get physical frame number of a giver virtual address using
 *          the Page table
 *
 *  @param addr Address to translate
 *    
 *  @return physical frame number or -1 if addr is not mapped 
 *          in physical memoru 
 * 
 */
int get_phys_frame(unsigned long addr) {
    
    pgd_t *pml4;
	pud_t *pdp;
	pmd_t *pde;
	pte_t *pte;
	
    int frame_number;

    // Page table virtual address
    pml4 = phys_to_virt(get_pt_addr());

    PRINT
    printk("%s: Page table address: 0x%p\n", MODNAME, pml4);

    
    
    // Check entries in the page tables

    //Check PML4
    if(!((unsigned long) pml4[PML4(addr)].pgd) & PRESENT_BIT) {
        PRINT
        printk("%s: No entry in PML4 for address: 0x%p\n", MODNAME, addr);
        return -1;
    }




    // Get PDP
    pdp = phys_to_virt((unsigned long)(pml4[PML4(addr)].pgd) & PT_ADDRESS_MASK);

    // Check PDP
    if(!((unsigned long) pdp[PDP(addr)].pud) & PRESENT_BIT) {
        PRINT
        printk("%s: No entry in PDP for address: 0x%p\n", MODNAME, addr);
        return -1;
    }
    else if(!((unsigned long) pdp[PDP(addr)].pud) & H_PAGES) {
        PRINT
        printk("%s: The entry 0x%p in PDP points to 1GB frame\n", MODNAME, addr);
        return -1;
    }
    




    // Get PDE
    pde = phys_to_virt((unsigned long) pdp[PDP(addr)].pud & PT_ADDRESS_MASK);

    // Check PDE
    if(!((unsigned long) pde[PDE(addr)].pmd) & PRESENT_BIT) {
        PRINT
        printk("%s: No entry in PDE for address: 0x%p\n", MODNAME, addr);
        return -1;
    }
    else if(!((unsigned long) pde[PDE(addr)].pmd) & H_PAGES) {
        PRINT
        printk("%s: The entry 0x%p in PDE points to 2MB frame\n", MODNAME, addr);
        
        frame_number = ((unsigned long)(pde[PDE(addr)].pmd) & PT_ADDRESS_MASK) >> 12;

        return frame_number;
    }





    // Get PTE
    pte = phys_to_virt((unsigned long) pde[PDE(addr)].pmd & PT_ADDRESS_MASK);

    // Check PTE
    if(!((unsigned long) pte[PTE(addr)].pte) & PRESENT_BIT) {
        PRINT
        printk("%s: No entry in PTE for address: 0x%p\n", MODNAME, addr);
        return -1;
    }

    frame_number = ((unsigned long)(pte[PTE(addr)].pte) & PT_ADDRESS_MASK) >> 12;


    return frame_number;
}

