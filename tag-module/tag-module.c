/**
 *  @file   tag-module.c
 *  @brief  Source code for the module relative to TAG-based message exchange
 *  @author Andrea Paci
 */ 


#include "module.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea Paci <andrea.paci1998@gmail.com");
MODULE_DESCRIPTION("TAG-based message exchange");




int init_module(void) {


    printk("%s: Mounting.\n", MODNAME);
    
    PRINT {
        printk("%s: Debug mode enabled ($MOD_DEBUG is set to 1)\n", MODNAME);
        printk("%s: Installing system calls\n", MODNAME);
    }

    install_syscalls();
    

    return 0;
}











void cleanup_module(void) {
    printk("%s: Unmounting.\n", MODNAME);

}
