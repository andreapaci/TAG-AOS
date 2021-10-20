/**
 *  @file   scth.c
 *  @brief  Source code for the module used for discovering the system call table position
 *          with its free entries at runtime 
 *  @author Andrea Paci
 */ 


#include "scth.h"



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea Paci <andrea.paci1998@gmail.com");
MODULE_DESCRIPTION("System call table discovery and hacking");


// If TEST_SYSCALL Macro is defined install a dummy system call
#ifdef TEST_SYSCALL

#include <linux/string.h>
#include <linux/slab.h>

#include "../utils/include/bitmask.h"
#include "../utils/include/hashmap.h"
#include "../utils/include/common.h"

#include "include/syscall-handle.h"

#define SEED0 401861
#define SEED1 879023
#define HASHMAP_CAP 256

void test_syscall(void);
int test_bitmask(void);
int test_hashmap(void);


__SYSCALL_DEFINEx(2, _trial, unsigned long, A, unsigned long, B){
        printk("%s: thread %d requests a trial sys_call with %lu and %lu as parameters\n",MODNAME,current->pid,A,B);

        test_syscall();

        return 0;
    }

unsigned long sys_trial;    
#endif  



int init_module(void) {


    printk("%s: Mounting.\n", MODNAME);
    PRINT
    printk("%s: Debug mode enabled ($MOD_DEBUG is set to 1)\n", MODNAME);
    #ifdef TEST_SYSCALL
    printk("%s: TEST_SYSCALL defined. A Dummy Syscall will be installed.\n", MODNAME);
    #endif


    find_syscall_table();

    // If TEST_SYSCALL Macro is defined install a dummy system call
    #ifdef TEST_SYSCALL
    printk("%s: Install dummy system call\n", MODNAME);
    sys_trial = (unsigned long) __x64_sys_trial;
    syscall_insert((unsigned long*) sys_trial);
    printk("%s: Dummy syscall installed.\n", MODNAME);

    #endif

    return 0;
}











void cleanup_module(void) {
    printk("%s: Unmounting.\n", MODNAME);

    // If TEST_SYSCALL Macro is defined delete the dummy system call
    #ifdef TEST_SYSCALL
    printk("%s: Removing dummy system call\n", MODNAME);
    #endif

    syscall_clean();
    
}






/**
 * 
[ 7885.601547] SCTH: Syscall table found at page18446744071880835072
               With address 18446744071880835840
[ 7885.601550] SCTH: Sys_ni has value 18446744071864075648
[ 7885.601551] SCTH: Entry 0 of ni_syscall: 18446744071864075648
[ 7885.601552] SCTH: Entry 1 of ni_syscall: 18446744071864075648
[ 7885.601552] SCTH: Entry 2 of ni_syscall: 18446744071864075648
[ 7885.601553] SCTH: Entry 3 of ni_syscall: 18446744071864075648
[ 7885.601553] SCTH: Entry 4 of ni_syscall: 18446744071864075648
[ 7885.601554] SCTH: Entry 5 of ni_syscall: 18446744071864075648
[ 7885.601554] SCTH: Entry 6 of ni_syscall: 18446744071864075648
[ 7885.628598] SCTH: Syscall table found at page18446744071885713408
               With address 18446744071885714704
[ 7885.628600] SCTH: Sys_ni has value 18446744071890792448
[ 7885.628601] SCTH: Entry 0 of ni_syscall: 18446744071890792448
[ 7885.628602] SCTH: Entry 1 of ni_syscall: 18446744071890792448
[ 7885.628602] SCTH: Entry 2 of ni_syscall: 18446744071890792448
[ 7885.628603] SCTH: Entry 3 of ni_syscall: 18446744071890792448
[ 7885.628604] SCTH: Entry 4 of ni_syscall: 18446744071890792448
[ 7885.628604] SCTH: Entry 5 of ni_syscall: 18446744071890792448
[ 7885.628605] SCTH: Entry 6 of ni_syscall: 18446744071890792448
[ 7885.647685] SCTH: Syscall table found at page18446744071889330176
               With address 18446744071889330584
[ 7885.647687] SCTH: Sys_ni has value 18446744071865873648
[ 7885.647688] SCTH: Entry 0 of ni_syscall: 18446744071865873648
[ 7885.647689] SCTH: Entry 1 of ni_syscall: 18446744071865873648
[ 7885.647689] SCTH: Entry 2 of ni_syscall: 18446744071865873648
[ 7885.647690] SCTH: Entry 3 of ni_syscall: 18446744071865873648
[ 7885.647690] SCTH: Entry 4 of ni_syscall: 18446744071865873648
[ 7885.647691] SCTH: Entry 5 of ni_syscall: 18446744071865873648
[ 7885.647691] SCTH: Entry 6 of ni_syscall: 18446744071865873648
[ 7885.682972] SCTH: Syscall table found at page18446744071896096768
               With address 18446744071896097032
[ 7885.682974] SCTH: Sys_ni has value 18446744071881500192
[ 7885.682975] SCTH: Entry 0 of ni_syscall: 18446744071881500192
[ 7885.682976] SCTH: Entry 1 of ni_syscall: 18446744071881500192
[ 7885.682976] SCTH: Entry 2 of ni_syscall: 18446744071881500192
[ 7885.682977] SCTH: Entry 3 of ni_syscall: 18446744071881500192
[ 7885.682977] SCTH: Entry 4 of ni_syscall: 18446744071881500192
[ 7885.682978] SCTH: Entry 5 of ni_syscall: 18446744071881500192
[ 7885.682978] SCTH: Entry 6 of ni_syscall: 18446744071881500192
[ 7885.750594] SCTH: Done

 * 
 * NEW VERSION
 * 
 * 
[11321.544587] SCTH: Syscall table found (Page addr: 18446744071880835072)
[11321.544590] SCTH: Syscall address 18446744071880835840
[11321.544591] SCTH: Sys_ni has value 18446744071864075648
[11321.544591] SCTH: Entry 0 of ni_syscall: 18446744071864075648
[11321.544592] SCTH: Entry 1 of ni_syscall: 18446744071864075648
[11321.544593] SCTH: Entry 2 of ni_syscall: 18446744071864075648
[11321.544593] SCTH: Entry 3 of ni_syscall: 18446744071864075648
[11321.544594] SCTH: Entry 4 of ni_syscall: 18446744071864075648
[11321.544595] SCTH: Entry 5 of ni_syscall: 18446744071864075648
[11321.544595] SCTH: Entry 6 of ni_syscall: 18446744071864075648
[11321.572644] SCTH: Syscall table found (Page addr: 18446744071885713408)
[11321.572648] SCTH: Syscall address 18446744071885714704
[11321.572648] SCTH: Sys_ni has value 18446744071890792448
[11321.572649] SCTH: Entry 0 of ni_syscall: 18446744071890792448
[11321.572650] SCTH: Entry 1 of ni_syscall: 18446744071890792448
[11321.572650] SCTH: Entry 2 of ni_syscall: 18446744071890792448
[11321.572651] SCTH: Entry 3 of ni_syscall: 18446744071890792448
[11321.572651] SCTH: Entry 4 of ni_syscall: 18446744071890792448
[11321.572652] SCTH: Entry 5 of ni_syscall: 18446744071890792448
[11321.572653] SCTH: Entry 6 of ni_syscall: 18446744071890792448
[11321.592408] SCTH: Syscall table found (Page addr: 18446744071889330176)
[11321.592411] SCTH: Syscall address 18446744071889330584
[11321.592412] SCTH: Sys_ni has value 18446744071865873648
[11321.592413] SCTH: Entry 0 of ni_syscall: 18446744071865873648
[11321.592413] SCTH: Entry 1 of ni_syscall: 18446744071865873648
[11321.592414] SCTH: Entry 2 of ni_syscall: 18446744071865873648
[11321.592415] SCTH: Entry 3 of ni_syscall: 18446744071865873648
[11321.592415] SCTH: Entry 4 of ni_syscall: 18446744071865873648
[11321.592416] SCTH: Entry 5 of ni_syscall: 18446744071865873648
[11321.592416] SCTH: Entry 6 of ni_syscall: 18446744071865873648
[11321.628525] SCTH: Syscall table found (Page addr: 18446744071896096768)
[11321.628528] SCTH: Syscall address 18446744071896097032
[11321.628528] SCTH: Sys_ni has value 18446744071881500192
[11321.628529] SCTH: Entry 0 of ni_syscall: 18446744071881500192
[11321.628530] SCTH: Entry 1 of ni_syscall: 18446744071881500192
[11321.628530] SCTH: Entry 2 of ni_syscall: 18446744071881500192
[11321.628531] SCTH: Entry 3 of ni_syscall: 18446744071881500192
[11321.628532] SCTH: Entry 4 of ni_syscall: 18446744071881500192
[11321.628532] SCTH: Entry 5 of ni_syscall: 18446744071881500192
[11321.628533] SCTH: Entry 6 of ni_syscall: 18446744071881500192
[11321.698032] SCTH: Done
 * 






 18446744072579187456
 18446744072562427264
 * 
 * 
 */ 


#ifdef TEST_SYSCALL


static __always_inline void *alloc(size_t size) { return kzalloc(size, GFP_ATOMIC); }
static __always_inline void dealloc(void* obj) { kfree(obj); }


void test_syscall(void) {

    printk("[TEST_FUNC]: Testing\n");

    printk("[TEST_FUNC]: Bitmask Testing\n");

    if(test_bitmask() == -1) printk("Error in bitmask testing\n");


    printk("[TEST_FUNC]: Hashmap Testing\n");

    if(test_hashmap() == -1) printk("Error in hashmap testing\n");

    printk("[TEST_FUNC]: All test executed correctly!\n");

}



int test_bitmask(void) {

    bitmask_t* mask = initialize_bitmask(0);

    if(mask == 0) printk("[TEST_FUNC]: Error initalizing mask with size 0, Correct!\n");
    else return -1;

    mask = initialize_bitmask(-1);

    if(mask == 0) printk("[TEST_FUNC]: Error initalizing mask with size -1, Correct!\n");
    else return -1;

    // NOTE: Automate this TEST FROM HERE --------------------------------

    mask = initialize_bitmask(1);
    if(mask == 0) return -1;
    
    printk("[TEST_FUNC]: Mask with 1 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 1 && mask -> n_bits == 1) printk("[TEST_FUNC]: Correct!\n");
    else{ printk("[TEST_FUNC]: Wrong!\n"); return -1; }

    
    free_bitmask(mask);
    


    mask = initialize_bitmask(63);
    if(mask == 0) return -1;
    
    printk("[TEST_FUNC]: Mask with 63 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 1 && mask -> n_bits == 63) printk("[TEST_FUNC]: Correct!\n");
    else{ printk("[TEST_FUNC]: Wrong!\n"); return -1; }
    
    free_bitmask(mask);




    mask = initialize_bitmask(64);
    if(mask == 0) return -1;
    
    printk("[TEST_FUNC]: Mask with 64 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 1 && mask -> n_bits == 64) printk("[TEST_FUNC]: Correct!\n");
    else{ printk("[TEST_FUNC]: Wrong!\n"); return -1; }
    
    free_bitmask(mask);



    mask = initialize_bitmask(65);
    if(mask == 0) return -1;
    
    printk("[TEST_FUNC]: Mask with 65 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 2 && mask -> n_bits == 65) printk("[TEST_FUNC]: Correct!\n");
    else{ printk("[TEST_FUNC]: Wrong!\n"); return -1; }
    
    free_bitmask(mask);

    // TO HERE ------------------------------------------------------------









    printk("\n[TEST_FUNC]: Getting new avaliable numbers\n");
    
    mask = initialize_bitmask(65);
    if(mask == 0) return -1;




    int number = get_avail_number(mask);
    if(number != 0){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu\n", number, *(mask -> mask));

    number = get_avail_number(mask);
    if(number != 1){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu\n", number, *(mask -> mask));

    number = get_avail_number(mask);
    if(number != 2){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu\n", number, *(mask -> mask));
    



    if(clear_number(mask, 1) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 1\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 1, Mask: %llu\n", *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 1){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu\n", number, *(mask -> mask));


    

    number = get_avail_number(mask);
    if(number != 3){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu\n", number, *(mask -> mask));

    number = get_avail_number(mask);
    if(number != 4){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu\n", number, *(mask -> mask));

    if(clear_number(mask, 2) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 2\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 2, Mask: %llu\n", *(mask -> mask));

    if(clear_number(mask, 3) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 2\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 3, Mask: %llu\n", *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 2){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu\n", number, *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 3){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu\n", number, *(mask -> mask));


    if(clear_number(mask, 4) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 4\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 4, Mask: %llu\n", *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 4){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu\n", number, *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 5){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu\n", number, *(mask -> mask));


    if(clear_number(mask, 4) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 4\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 4, Mask: %llu\n", *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 4){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu\n", number, *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 6){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu\n", number, *(mask -> mask));






    printk("\n[TEST_FUNC]: Clearing numbers\n");

    if(clear_number(mask, 12) != 0) {
        printk("[TEST_FUNC]: Error in clearing number: 12\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 12, Mask: %llu\n", *(mask -> mask));

    if(clear_number(mask, 63) != 0) {
        printk("[TEST_FUNC]: Error in clearing number: 63\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 63, Mask: %llu\n", *(mask -> mask));

    if(clear_number(mask, 64) != 0) {
        printk("[TEST_FUNC]: Error in clearing number: 64\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 64, Mask: %llu\n", *(mask -> mask));

    if(clear_number(mask, 65) != -1) {
        printk("[TEST_FUNC]: Error in clearing number: 65\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 65, Mask: %llu\n", *(mask -> mask));

    if(clear_number(mask, 66) != -1) {
        printk("[TEST_FUNC]: Error in clearing number: 66\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 66, Mask: %llu\n", *(mask -> mask));

    printk("[TEST_FUNC]: Test Get/Clear free numbers executed correctly!\n");

    free_bitmask(mask);


    
    
    
    
    printk("\n[TEST_FUNC]: Test Maximum numbers allowed!\n");

    mask = initialize_bitmask(65);
    if(mask == 0) return -1;

    printk("[TEST_FUNC]: Mask with 65 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 2 && mask -> n_bits == 65) printk("[TEST_FUNC]: Correct!\n");
    else{ printk("[TEST_FUNC]: Wrong!\n"); return -1; }

    int i = 0;
    for(; i < 65; i++) {

        int number;
        unsigned long long cycle;

        cycle = rdtsc_fenced();

        number = get_avail_number(mask);

        cycle = rdtsc_fenced() - cycle;


        printk("\t Time to get free number = %llu\n", cycle);

        
        if(number != i){ 
            printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
            return -1;
        }
        printk("[TEST_FUNC]: Number: %d, Mask: %llu, Mask#2: %llu\n", number, *((mask -> mask) + 1), *(mask -> mask));
    
    }

    number = get_avail_number(mask);
    if(number != -1){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d Not allowed, Correct!, Mask: %llu, Mask#2: %llu\n", number, *((mask -> mask) + 1), *(mask -> mask));


    if(clear_number(mask, 54) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 54\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 54, Mask: %llu, Mask#2: %llu\n", *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 54){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu, Mask#2: %llu\n", number, *((mask -> mask) + 1), *(mask -> mask));

    free_bitmask(mask);



    printk("\n[TEST_FUNC]: Test Multiple slots!\n");


    mask = initialize_bitmask(134);
    if(mask == 0) return -1;

    printk("[TEST_FUNC]: Mask with 134 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 3 && mask -> n_bits == 134) printk("[TEST_FUNC]: Correct!\n");
    else{ printk("[TEST_FUNC]: Wrong!\n"); return -1; }

    i = 0;
    for(; i < 134; i++) {
        
        int number;
        unsigned long long cycle;

        cycle = rdtsc_fenced();

        number = get_avail_number(mask);

        cycle = rdtsc_fenced() - cycle;


        printk("\t Time to get free number = %llu\n", cycle);

        if(number != i){ 
            printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
            return -1;
        }
        printk("[TEST_FUNC]: Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));
    
    }

    number = get_avail_number(mask);
    if(number != -1){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d Not allowed, Correct!, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));





    if(clear_number(mask, 54) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 54\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 54, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 54){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));




    if(clear_number(mask, 63) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 63\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 63, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 63){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));




    if(clear_number(mask, 64) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 64\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 64, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 64){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));




    if(clear_number(mask, 65) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 65\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 65, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 65){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));





    if(clear_number(mask, 67) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 67\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 67, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 67){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));





    if(clear_number(mask, 127) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 127\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 127, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 127){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));





    if(clear_number(mask, 128) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 128\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 128, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 128){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));






    if(clear_number(mask, 130) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 130\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 130, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 130){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));

    
    
    
    if(clear_number(mask, 133) != 1) {
        printk("[TEST_FUNC]: Error in clearing number: 133\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 133, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 133){ 
        printk("[TEST_FUNC]: Error in getting new number: %d\n", number);
        return -1;
    }
    printk("[TEST_FUNC]: Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));

    




    if(clear_number(mask, 134) != -1) {
        printk("[TEST_FUNC]: Error in clearing number: 134\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 134, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));

    if(clear_number(mask, 135) != -1) {
        printk("[TEST_FUNC]: Error in clearing number: 135\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 135, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));

    if(clear_number(mask, 344) != -1) {
        printk("[TEST_FUNC]: Error in clearing number: 344\n");
        return -1;
    }
    printk("[TEST_FUNC]: Cleared 344, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));

    

    free_bitmask(mask);



   

    printk("[TEST_FUNC]: Test Bitmask executed correctly!\n");
    return 0;

}

typedef struct data { 
    int key;
    char* buffer;
} data;

int compare_hash(const void* a, const void* b, void* udata) {
    
    return ((data *) a) -> key - ((data *) b) -> key;
}

uint64_t custom_hash(const void *item, uint64_t seed0, uint64_t seed1 ) {

    const data* entry = item;
    return hashmap_sip(&(entry->key), sizeof(int), seed0, seed1);
}

int test_hashmap(void) {

    // Initialize hashmap which stores string (char[16])
    struct hashmap* map = hashmap_new_with_allocator(
        alloc, 0, dealloc, sizeof(data), 
        HASHMAP_CAP, SEED0, SEED1, 
        custom_hash, compare_hash, 0);
    if(map == 0) {
        printk("[TEST_FUNC]: Hashamp non initalized!\n");
        return -1;
    }

    data data_ins = *((data *)alloc(sizeof(data)));
    data_ins.key = 1;
    data_ins.buffer = "chiave1";

    printk("[TEST_FUNC]: Value: %d, %s\n", data_ins.key, data_ins.buffer);


    printk("[TEST_FUNC]: Hashmap initalized\n");

    hashmap_set(map, &data_ins);

    printk("[TEST_FUNC]: Add 1 value\n");

    hashmap_set(map, &(data){ .key=2, .buffer="chiave2"});

    printk("[TEST_FUNC]: Add 1 value\n");

    hashmap_set(map, &(data){ .key=3, .buffer="chiave3"});

    printk("[TEST_FUNC]: Add 1 value\n");


    printk("[TEST_FUNC]: Added %ld values\n", hashmap_count(map));

    if(hashmap_count(map) != 3) {
        printk("[TEST_FUNC]: Number of entries should be 3!\n");
        return -1;
    }


    int key_ret = 1;
    for(; key_ret < 4; key_ret++){
        data *data_ret = hashmap_get(map, &key_ret);
        if(data_ret == 0) {
             printk("[TEST_FUNC]: Error getting data %d\n", key_ret);
             return -1;
        }
        printk("[TEST_FUNC]:\tData %d: %d, %s\n", key_ret, data_ret -> key, data_ret -> buffer);

    }
    printk("[TEST_FUNC]: 3 values added correctly\n");


    key_ret = 4;
    data* data_ret = hashmap_get(map, &key_ret);
    if(data_ret != 0) {
        printk("[TEST_FUNC]: Returned non-existing value: %d, %s\n", data_ret -> key, data_ret -> buffer);
        return -1;
    }

    printk("[TEST_FUNC]: Non existing value not returned, correct\n");

    data_ret = hashmap_set(map, &(data){ .key=1, .buffer="nuovachiave"});
    if(data_ret == 0) {
         printk("[TEST_FUNC]: No value replaced for key 1!\n");
         return -1;
    } 
    if(data_ret -> key != 1 || strcmp(data_ret -> buffer, "chiave1") != 0) {
        printk("[TEST_FUNC]: Wrong value replaced for key 1: %d, %s\n", data_ret -> key, data_ret -> buffer);
        return -1;
    }

    printk("[TEST_FUNC]: Old value replaced: %d, %s\n", data_ret -> key, data_ret -> buffer);


    key_ret = 1;
    data_ret = hashmap_get(map, &key_ret);
    if(data_ret == 0) {
         printk("[TEST_FUNC]: No value returned for key 1!\n");
         return -1;
    } 
    if(data_ret -> key != 1 || strcmp(data_ret -> buffer, "nuovachiave") != 0) {
        printk("[TEST_FUNC]: Wrong value returned: %d, %s\n", data_ret -> key, data_ret -> buffer);
        return -1;
    }
    printk("[TEST_FUNC]: Value returned: %d, %s\n", data_ret -> key, data_ret -> buffer);

    key_ret = 4;
    data_ret = hashmap_delete(map, &key_ret);
    if(data_ret != 0) {
        printk("[TEST_FUNC]: Deleted non-existing \n");
        return -1;
    }
    printk("[TEST_FUNC]: Non-existing not deleted, Correct!\n");


    key_ret = 2;
    data_ret = hashmap_delete(map, &key_ret);
    if(data_ret == 0) {
         printk("[TEST_FUNC]: No value returned for key 2!\n");
         return -1;
    } 
    if(data_ret -> key != 2 || strcmp(data_ret -> buffer, "chiave2") != 0) {
        printk("[TEST_FUNC]: Wrong value deleted: %d, %s\n", data_ret -> key, data_ret -> buffer);
        return -1;
    }
    printk("[TEST_FUNC]: Value deleted: %d, %s\n", data_ret -> key, data_ret -> buffer);

    printk("[TEST_FUNC]: Testing 260 values\n");

    hashmap_set(map, &(data){ .key=2, .buffer="chiave2"});

    int tries, i;
    tries = 1556;
    i = 1;
    

    preempt_disable();

    for(; i < tries; i++) {

        char* number = alloc(sizeof(char) * 20);
        if(number == 0) { printk("[TEST_FUNC]: Error in allocating buffer memory for %d\n", i); return -1;}
        sprintf(number,"chiave%d", i);

        unsigned long long cycle;
        
        cycle = rdtsc_fenced();

        hashmap_set(map, &(data){ .key=i, .buffer=number});

        cycle = rdtsc_fenced() - cycle;


        printk("\t Time to set = %llu\n", cycle);


        cycle = rdtsc_fenced();

        data_ret = hashmap_get(map, &i);

        cycle = rdtsc_fenced() - cycle;


        printk("\t Time to get = %llu\n", cycle);
        if(data_ret == 0) {
            printk("[TEST_FUNC]: No value returned for key %d!\n", i);
            return -1;
        } 
        printk("[TEST_FUNC]: Value returned: %d, %s\n", data_ret -> key, data_ret -> buffer);
        
        if(data_ret -> key != i || strcmp(data_ret -> buffer, number) != 0) {
            printk("[TEST_FUNC]: Wrong value returned\n");
            return -1;
        }
        printk("[TEST_FUNC]: Correct %d\n", i);
    
    }

    preempt_enable();

    printk("[TEST_FUNC]: Freeing space\n");

    i = 1;
    for(; i < tries; i++) {

        data_ret = (data *) hashmap_delete(map, &i);
        if(data_ret == 0) { printk("[TEST_FUNC]: Error in deleting element %d\n", i); return -1; }
        printk("[TEST_FUNC]: Value deleted: %d, %s\n", data_ret -> key, data_ret -> buffer);
        dealloc(data_ret -> buffer);
        
    }

    hashmap_free(map);    

    printk("[TEST_FUNC]: Test Hashmap executed correctly!\n");

    return 0;





}



#endif