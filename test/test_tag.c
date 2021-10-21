#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "tag.h"


#ifndef SYS_gettid
#error "SYS_gettid unavailable on this system"
#endif

#define gettid() ((pid_t)syscall(SYS_gettid))

void* thread_fun();
void* thread_rcv();
void  create_del_test();

int main(int argc, char** argv) {

    


    pthread_t tid;

    int ret;
    ret = pthread_create(&tid, 0, thread_fun, 0);

    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        exit(ret);
    }

    printf("Main thread PID: %d\n", getpid());
    printf("Main thread TID: %d\n", gettid());

    create_del_test();
    
    pthread_join(tid, 0);

    printf("----------------------------------------\n");

    pthread_t tid_a[54];
    for(int i = 0; i < 54; i++) {
        ret = pthread_create(&tid_a[i], 0, thread_rcv, 0);

        if(ret != 0) {
            printf("Error creating thread, error: %d\n", ret);
            exit(ret);
        }
    }


    /*char buffer[20];

    printf("Send message: \n");
    scanf("%s", buffer);
    printf("Ok\n");
    */
    printf("Main - Opening %d\n", tag_get(4, TAG_CREAT, 1));
    
    sleep(10);

    tag_send(0, 1, 0, 0);
    
    printf("Main - Send Done\n");

    sleep(10);
    
    printf("Main - CTL\n");

    tag_ctl(0, TAG_AWAKE_ALL);

    printf("Main - CTL Done\n");
    
    for(int i = 0; i < 54; i++) {
        
        pthread_join(tid_a[i], 0);
    
    }
    printf("All Done\n");

}


void* thread_fun(){

    printf("Created thread PID: %d\n", getpid());
    printf("Created thread TID: %d\n", gettid());
}

void  create_del_test() {

    printf("tag_get: %d\n", tag_get(1,TAG_CREAT,1));
    printf("tag_get: %d\n", tag_get(1,TAG_CREAT,1));
    printf("tag_get: %d\n", tag_get(1,TAG_OPEN,1));
    
    printf("tag_get: %d\n", tag_get(2,TAG_CREAT,1));
    printf("tag_get: %d\n", tag_get(3,TAG_CREAT,1));

    printf("tag_ctl: %d\n", tag_ctl(0,TAG_DELETE));

    printf("tag_get: %d\n", tag_get(1,TAG_OPEN,1));
    printf("tag_get: %d\n", tag_get(1,TAG_CREAT,1));

    printf("tag_ctl: %d\n", tag_ctl(0,TAG_DELETE));
    printf("tag_ctl: %d\n", tag_ctl(1,TAG_DELETE));
    printf("tag_ctl: %d\n", tag_ctl(2,TAG_DELETE));

    
    printf("tag_get: %d\n", tag_get(1,TAG_OPEN,1));
    printf("tag_get: %d\n", tag_get(24,TAG_OPEN,1));

    printf("tag_get: %d\n", tag_get(24,TAG_CREAT,1));

    printf("tag_ctl: %d\n", tag_ctl(0,TAG_DELETE));
    printf("tag_ctl: %d\n", tag_ctl(16,TAG_DELETE));
}

void* thread_rcv(){

    int tid = gettid() % 54;

    sleep(5);
    printf("%d - Opening tag: %d\n", tid, tag_get(4, TAG_OPEN, 1));

    sleep(2);
    
    tag_receive(0, 1, 0, 0);

    printf("%d - Receive Done\n", tid );

    printf("%d - Receiving on random level\n", tid);

    tag_receive(0, tid % (30) , 0, 0);

    printf("%d - Done\n", tid);

}


