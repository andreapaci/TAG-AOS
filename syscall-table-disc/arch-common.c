/**
 *  @file   arch-common.c
 *  @brief  Basic utilities to interact with architectural-dependent
 *          components using machine dependent code 
 *  @author Andrea Paci
 */ 


#include "arch-common.h"

unsigned long __force_order;



/**
 *  @brief  Get page table address from CR3 register, masking the register
 *          value
 *    
 *  @return physical address of Page Table (PML4 in x86-64 Long Mode)  
 * 
 */
inline unsigned long long get_pt_addr(void) {
    unsigned long long addr;
    asm volatile("mov %%cr3,%0":  "=r" (addr) : );
    return addr & PT_CR3_MASK;
}



/**
 *  @brief  Disable WP bit in CR0
 *  
 *  @param  flags for IRQ State save
 * 
 */
inline void disable_WP(unsigned long* flags, unsigned long cr0) {
    
    local_irq_save(*flags);
    preempt_disable();

    unsigned long modified_cr0 = cr0 & ~X86_CR0_WP;

    PRINT
    printk("%s: Disabling Write Protection Bit in CR0", MODNAME);

    asm volatile("mov %0, %%cr0":  "+r"(modified_cr0), "+m"(__force_order));
    asm volatile("sfence":::"memory");

}

/**
 *  @brief  Enable WP bit in CR0
 * 
 *  @param  flags for IRQ State save
 * 
 */
inline void enable_WP(unsigned long* flags, unsigned long cr0) {
    
    PRINT
    printk("%s: Enabling Write Protection Bit in CR0", MODNAME);
    
    asm volatile("mov %0, %%cr0":  "+r" (cr0), "+m"(__force_order) );

    preempt_enable();
    local_irq_restore(*flags);
    
    asm volatile("mfence":::"memory");
    
}
