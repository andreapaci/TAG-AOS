/**
 *  @file   pte.c
 *  @brief  Source code for Page Table Explorer in a x86-64 system and checking
 *          wether a specific virtual address is mapped in physical memory 
 *  @author Andrea Paci
 */ 


#include "ptexplorer.h"

#define PML4(addr) (((unsigned long long)(addr) >> 39) & 0x1ffULL)
#define PDP(addr)  (((unsigned long long)(addr) >> 30) & 0x1ffULL)
#define PDE(addr)  (((unsigned long long)(addr) >> 21) & 0x1ffULL)
#define PTE(addr)  (((unsigned long long)(addr) >> 12) & 0x1ffULL)

#define PRESENT_BIT     ((unsigned long long) 0x1ULL)                 // Mask to check the Present bit
#define PT_ADDRESS_MASK ((unsigned long long) 0x7ffffffffffff000ULL)  // Mask to get the subsequent page table address (or huge page)
#define H_PAGES         ((unsigned long long) 0x80ULL)                // Mask to check wether the PT entry points to another PT or a frame (1GB/2MB)                               


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
unsigned long long get_phys_frame(unsigned long long addr) {
    
    pgd_t *pml4;
	pud_t *pdp;
	pmd_t *pde;
	pte_t *pte;
	
    int frame_number;

    // Page table virtual address
    pml4 = __va(get_pt_addr());
    
    
    // Check entries in the page tables

    //Check PML4
    if(!((pml4[PML4(addr)].pgd) & PRESENT_BIT)) return -1;



    // Get PDP
    pdp = __va((pml4[PML4(addr)].pgd) & PT_ADDRESS_MASK);

    // Check PDP
    if(!((pdp[PDP(addr)].pud) & PRESENT_BIT)) {
        return -1;
    }
    else if((pdp[PDP(addr)].pud) & H_PAGES) {
        return -1;
    }
    


    // Get PDE
    pde = __va(pdp[PDP(addr)].pud & PT_ADDRESS_MASK);

    // Check PDE
    if(!((pde[PDE(addr)].pmd) & PRESENT_BIT)) {
        return -1;
    }
    else if((pde[PDE(addr)].pmd) & H_PAGES) {
        
        frame_number = (unsigned long long)((pde[PDE(addr)].pmd) & PT_ADDRESS_MASK) >> 12;

        return frame_number;
    }



    // Get PTE
    pte = __va((unsigned long) pde[PDE(addr)].pmd & PT_ADDRESS_MASK);

    // Check PTE
    if(!((pte[PTE(addr)].pte) & PRESENT_BIT)) {
        return -1;
    }

    frame_number = ((unsigned long long)(pte[PTE(addr)].pte) & PT_ADDRESS_MASK) >> 12;

    return frame_number;
}

