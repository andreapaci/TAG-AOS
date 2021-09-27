/**
 *  @file   tag-syscall.c
 *  @brief  Source code for the definition of the system calls relative to TAG module
 *  @author Andrea Paci
 */ 


#include "module.h"


int tag_get(int key, int command, int permission) {

    PRINT
    printk("%s: Tag get has been called.\n", MODNAME);

    return 0;
}

int tag_send(int tag, int level, char* buffer, size_t size) { 

    PRINT
    printk("%s: Tag send has been called.\n", MODNAME);

    
    return 1;
}

int tag_receive(int tag, int level, char* buffer, size_t size) { 


    PRINT
    printk("%s: Tag receive has been called.\n", MODNAME);

    return 2;
}

int tag_ctl(int tag, int command) {

    PRINT
    printk("%s: Tag ctl has been called.\n", MODNAME);


    return 3;
}


__SYSCALL_DEFINEx(3, _tag_get, int, key, int, command, int, permission){ 
        return tag_get(key, command, permission);
}

unsigned long sys_tag_get;


__SYSCALL_DEFINEx(4, _tag_send, int, tag, int, level, char*, buffer, size_t, size) {
        return tag_send(tag, level, buffer, size);
}

unsigned long sys_tag_send;    


__SYSCALL_DEFINEx(4, _tag_receive, int, tag, int, level, char*, buffer, size_t, size) {
        return tag_receive(tag, level, buffer, size);
}

unsigned long sys_tag_receive;    


__SYSCALL_DEFINEx(2, _tag_ctl, int, tag, int, command) {
        return tag_ctl(tag, command);
}

unsigned long sys_tag_ctl;    


int install_syscalls(void) {
    
    sys_tag_get     = (unsigned long) __x64_sys_tag_get;
    sys_tag_send    = (unsigned long) __x64_sys_tag_send;
    sys_tag_receive = (unsigned long) __x64_sys_tag_receive;
    sys_tag_ctl     = (unsigned long) __x64_sys_tag_ctl;

    return
    syscall_insert((unsigned long *) sys_tag_get)       *
    syscall_insert((unsigned long *) sys_tag_send)      *
    syscall_insert((unsigned long *) sys_tag_receive)   *
    syscall_insert((unsigned long *) sys_tag_ctl);

}






