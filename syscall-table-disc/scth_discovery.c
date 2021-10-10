/**
 *  @file   scth_disc.c
 *  @brief  Source code for discovering the position of the System Call Table 
 *  @author Andrea Paci
 */ 



#include "scth.h"
#include "include/syscall-handle.h"

unsigned long long* syscall_table_addr = 0;
unsigned long long  sys_ni_address = 0;
const unsigned long long ni_syscall[] =	
                            { 134ull, 174ull, 182ull, 183ull, 214ull, 215ull, 236ull };
static int syscall_table_pattern(unsigned long long addr);

/**
 *  @brief  Find the system call table
 *    
 */
void find_syscall_table(void) {

    unsigned long long page;
    unsigned long long current_pg;

    for(page = START_ADDR; page < END_ADDR; page += PAGE_SIZE_DEF) {

        current_pg = page;

        // Check if "page" is mapped
        if(get_phys_frame(current_pg) != -1ull) {

            
            // If mapped, search for syscall table

            if(syscall_table_pattern(current_pg)) {
                
                PRINT {
                    printk("%s: Syscall table found (Page addr: %llu)\n", MODNAME, current_pg);
                    printk("%s: Syscall address %llu\n", MODNAME, (unsigned long long) syscall_table_addr);
                    printk("%s: Sys_ni has value %llu\n", MODNAME, sys_ni_address);
                    int i = 0;
                    for(; i < FREE_ENTRIES; i++) {
                        printk("%s: Entry %d of ni_syscall: %llu\n", 
                        MODNAME, i, syscall_table_addr[ni_syscall[i]]);
                    }
                } else {
                    printk("%s: Syscall table found at address %llu\n", MODNAME, (unsigned long long) syscall_table_addr);
                }

                return;
            }

        }

    }

    PRINT
    printk("%s: Done\n", MODNAME); 

}


/**
 *  @brief  Check in a given address if there's a pattern equal to
 *          the one of the system call table
 * 
 *  @param  addr base address to use for pattern recognition (page aligned)
 * 
 *  @return 0 if no system call table was found, 1 otherwise
 *    
 */
static int syscall_table_pattern(unsigned long long addr) {


    unsigned long long offs;

    for(offs = addr; offs < addr + PAGE_SIZE_DEF; offs += 1ull) {
        
        unsigned long long next_page = 
                offs + ni_syscall[FREE_ENTRIES - 1] * ((unsigned long long) sizeof(void*));
        
        // Check if the last ni_syscall entry is on the subsequent page
        if(((offs & PAGE_BITMASK) + PAGE_SIZE_DEF) == (next_page & PAGE_BITMASK)) 
            if(get_phys_frame(next_page) == -1) return 0;


        unsigned long long* start_tb = (unsigned long long *) offs;
        unsigned long long first_ni_syscall = start_tb[ni_syscall[0]];
        
        //Check if first_ni_syscall is a good fit as sys_ni function pointer
        if(((first_ni_syscall & 0x3) == 0) &&
            (first_ni_syscall != 0x0)      &&
            (first_ni_syscall > START_ADDR)) {
                
                //Used to check conditions
                int condition_check = 0;
                
                // Pattern check
                int i;
                for(i = 1; i < FREE_ENTRIES; i++){
                    unsigned long long entry = start_tb[ni_syscall[i]];
                    if(entry != first_ni_syscall) condition_check = 1;
                }
                if(condition_check) continue;

                // Check if the area before first_ni_syscall points to ni_syscall
                int j = 1;
                for(; j < (int)ni_syscall[0]; j++)  {
                    if( start_tb[j] == start_tb[ni_syscall[0]]) condition_check = 1;
                }
                if(condition_check) continue; 

                syscall_table_addr = offs;
                sys_ni_address = start_tb[ni_syscall[0]];

                return 1;
                
        }
        
    }

    return 0;


}

/**
 *  @brief  Insert syscall_function inside the System Call Table
 * 
 *  @param syscall_fuction pointer to the function to add
 * 
 *  @return 0 if insertion was not successful, 1 otherwise
 *    
 */
int syscall_insert(unsigned long* syscall_function) {

    if(syscall_table_addr == 0 || sys_ni_address == 0) {
        printk("%s: System Call table not avaliable. Skipping installation\n", MODNAME);
        return 0;
    }

    unsigned long long* insertion_address = 0;
    unsigned int        displacement = 0;

    int i = 0;
    for(; i < FREE_ENTRIES; i++)
        if(syscall_table_addr[ni_syscall[i]] == sys_ni_address) {
            insertion_address = &syscall_table_addr[ni_syscall[i]];
            displacement = i;
            break;
        }    
    
    if(insertion_address == 0) {
        printk("%s: Error inserting Custom Syscall: no free entry found in the system call table\n", MODNAME);
        return 0;
    }
    
    unsigned long   flags;
    unsigned long   cr0 = read_cr0();

    disable_WP(&flags, cr0);
    *insertion_address = syscall_function;
    enable_WP(&flags, cr0);

    printk("%s: Custom Syscall has been installed at address %llu with displacement %llu\n", 
            MODNAME, &syscall_table_addr[ni_syscall[displacement]], ni_syscall[displacement]);
    
    return 1;       
}
EXPORT_SYMBOL(syscall_insert);



/**
 *  @brief  Cleaning of the system call table, re-establishing sys_ni 
 *          values modified entries
 * 
 */
void syscall_clean(void) {

    if(syscall_table_addr == 0 || sys_ni_address == 0) {
        printk("%s: System Call table not avaliable. Skipping restore.\n", MODNAME);
        return;
    }

    unsigned long   flags;
    unsigned long   cr0 = read_cr0();
    
    disable_WP(&flags, cr0);

    int i = 0;
    for(; i < FREE_ENTRIES; i++)
        syscall_table_addr[ni_syscall[i]] = sys_ni_address;

    enable_WP(&flags, cr0);
    
    printk("%s: System call table restored\n", MODNAME);
            
}








