#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "tag.h"

void* thread_fun();

int main(int argc, char** argv) {

    printf("tag_get: %d\n", tag_get(1,1,1));
    printf("tag_send: %d\n", tag_send(1,1,NULL, sizeof(void)));
    printf("tag_receive: %d\n", tag_receive(1,1,NULL, sizeof(void)));
    printf("tag_ctl: %d\n", tag_ctl(1,1));


    pthread_t tid;

    int ret;
    ret = pthread_create(&tid, 0, thread_fun, 0);

    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        exit(ret);
    }

    printf("Main thread PID: %d\n", getpid());

    pthread_join(tid, 0);


}


void* thread_fun(){

    printf("Created thread PID: %d\n", getpid());

}