/**
 *  @file   tag-syscall.h
 *  @brief  Header file containing the definition of TAG Module system calls
 *  @author Andrea Paci
 */ 

#include <sys/syscall.h>
#include <unistd.h>
#include "../tag-module/include/tag.h"

#ifndef TAG_GET_NR
#warning "tag_get() syscall number not defined"
#define TAG_GET_NR 134
#endif

#ifndef TAG_SEND_NR
#warning "tag_send() syscall number not defined"
#define TAG_SEND_NR 174
#endif

#ifndef TAG_RECEIVE_NR
#warning "tag_receive() syscall number not defined"
#define TAG_RECEIVE_NR 182
#endif

#ifndef TAG_CTL_NR
#warning "tag_ctl() syscall number not defined"
#define TAG_CTL_NR 183
#endif

int tag_get(int key, int command, int permission) {
    return syscall(TAG_GET_NR, key, command, permission);
}

int tag_send(int tag, int level, char* buffer, size_t size) {
    return syscall(TAG_SEND_NR, tag, level, buffer, size);
}

int tag_receive(int tag, int level, char* buffer, size_t size) {
    return syscall(TAG_RECEIVE_NR, tag, level, buffer, size);
}

int tag_ctl(int tag, int command) {
    return syscall(TAG_CTL_NR, tag, command);
}