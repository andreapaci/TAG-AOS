/**
 *  @file   scth.c
 *  @brief  Source code for the module used for discovering the system call table position
 *          with its free entries at runtime 
 *  @author Andrea Paci
 */ 


#include "scth.h"
#include "syscall-handle.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea Paci <andrea.paci1998@gmail.com");
MODULE_DESCRIPTION("System call table discovery and hacking");


// If TEST_SYSCALL Macro is defined install a dummy system call
#ifdef TEST_SYSCALL

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


void test_syscall(void) {





}



#endif