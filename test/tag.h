/**
 *  @file   tag-syscall.h
 *  @brief  Header file containing the definition of TAG Module system calls
 *  @author Andrea Paci
 */ 

#include <sys/syscall.h>
#include <unistd.h>
#include "../tag-module/include/tag.h"

#ifndef __NR_tag_get
#define __NR_tag_get 134
#endif

#ifndef __NR_tag_send
#define __NR_tag_send 174
#endif

#ifndef __NR_tag_receive
#define __NR_tag_receive 182
#endif

#ifndef __NR_tag_ctl
#define __NR_tag_ctl 183
#endif

int tag_get(int key, int command, int permission) {
    return syscall(__NR_tag_get, key, command, permission);
}

int tag_send(int tag, int level, char* buffer, size_t size) {
    return syscall(__NR_tag_send, tag, level, buffer, size);
}

int tag_receive(int tag, int level, char* buffer, size_t size) {
    return syscall(__NR_tag_receive, tag, level, buffer, size);
}

int tag_ctl(int tag, int command) {
    return syscall(__NR_tag_ctl, tag, command);
}