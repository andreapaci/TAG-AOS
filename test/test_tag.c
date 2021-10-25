/**
 *  @file   test_tag.c
 *  @brief  Source code for testing the TAG Module, trying to test the behaviour of the module
 *          in both normal use and borderline scenarios. The test can be executed with the "taskset" in order
 *          to achive Single Core execution and verify that no deadlock occurs.
 *          The test requires minimum interaction with the user, to further achieve debug information
 *          is possible to enable the DEBUG mode of the modules and have extra printed info using "dmesg"
 *  @author Andrea Paci
 */ 



#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>
#include "tag.h"


#ifndef SYS_gettid
#error "SYS_gettid unavailable on this system"
#endif

#define gettid() ((pid_t)syscall(SYS_gettid))

typedef struct input_struct {
    int key;
    int tag;
    int level;
    int size;
} input_t;


void interrupt_handler(int sig){
    printf("Signal received %d\n", sig);
}

void* receive_thread(void* input);
void* awake_thread(void* input);
void* delete_thread(void* input);


int test_tag_get();

int test_ipc_private();

int test_busy_ctl_interrupt();

void* thread_fun();
void* thread_rcv();
void  create_del_test();

int main(int argc, char** argv) {
    
    signal(SIGINT, interrupt_handler);
    
    printf("Testing various Tag functionality.\nMost of the debugging output will be visible with 'dmesg'\n\n");

    if(!test_tag_get()) return -1;

    printf("Test with tag_get() executed Succesfully! Press Enter to continue\n(check 'dmesg' for log if needed and possibly clear the log)\n");
    
    getchar();

    if(!test_ipc_private()) return -1;

    printf("Test with tag_get() and IPC_PRIVATE executed Succesfully! Press Enter to continue\n(check 'dmesg' for log if needed and possibly clear the log)\n");
    
    getchar();
    
    if(!test_busy_ctl_interrupt()) return -1;

    printf("Test with tag_receive() and tag_ctl() executed Succesfully! Press Enter to continue\n(check 'dmesg' for log if needed and possibly clear the log)\n");
    
    getchar();

    /*test_failed_closing();

    test_next_epoch();

    test_multiple_size();

    test_ctl_awake();

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

*/
    /*char buffer[20];

    printf("Send message: \n");
    scanf("%s", buffer);
    printf("Ok\n");
    */
  /*  printf("Main - Opening %d\n", tag_get(4, TAG_CREAT, 1));
    
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
    */
}

int test_tag_get() {

    printf("\nTesting multiple 'tag_get' in create (TID %d)\n\n", gettid());
    
    int test_tags, max_tags, i, ret_val;
    test_tags = 300; 
    max_tags = 256;

    printf("\nCreating %d tag services\n", test_tags);
    for(i = 0; i < test_tags; i++) {
        ret_val = tag_get((i + 1) * 3, TAG_CREAT, TAG_PERM_ALL);
        if(i < max_tags && ret_val == i)
            printf("Correct! tag_get (key %d) returned tag_descr %d\n", (i + 1) *3, ret_val);
        else if(i >= max_tags && ret_val < 0) 
            printf("Correct! Not possible to create more tags (max reached) (ret: %d)\n", ret_val);
        else {
            printf("Error for index %d (ret_val %d)\n", i, ret_val);
            return 0;
        }
    }

    printf("\nOpening all previously created tag services\n");
    for(i = 0; i < test_tags; i++) {
        ret_val = tag_get((i + 1) * 3, TAG_OPEN, TAG_PERM_ALL);
        if(i < max_tags && ret_val == i)
            printf("Correct! tag_get (key %d) returned tag_descr %d\n", (i + 1) * 3, ret_val);
        else if(i >= max_tags && ret_val < 0) 
            printf("Correct! Not possible to open unexisting tag (ret: %d)\n", ret_val);
        else {
            printf("Error for index %d (ret_val %d)\n", i, ret_val);
            return 0;
        }
    }


    printf("\nDelete all instances\n");
    for(i = 0; i < test_tags; i++) {
        ret_val = tag_ctl(i, TAG_DELETE);
        if(i < max_tags && ret_val == 1)
            printf("Correct! tag_ctl (delete) (tag %d) deleted succesfully\n", i);
        else if(i >= max_tags && ret_val < 0) 
            printf("Correct! Not possible to delete unexisting tag %d (ret: %d)\n", i, ret_val);
        else {
            printf("Error for index %d (ret_val %d)\n", i, ret_val);
            return 0;
        }
    }

    printf("\nTesting multiple 'tag_get' in create Done\n\n");


    return 1;
}

int test_ipc_private() {
    int test_tags, i, ret_val;
    test_tags = 7; 
    
    printf("\nTesting multiple 'tag_get' in create with IPC_PRIVATE as key (TID %d)\n\n", gettid());

    printf("\nCreate %d tag services with IPC_PRIVATE\n", test_tags);
    for(i = 0; i < test_tags; i++) {
        // IPC_PRIVATE == 0
        ret_val = tag_get(0, TAG_CREAT, TAG_PERM_ALL);
        if(ret_val == i)
            printf("Correct! tag_get (key %d) returned tag_descr %d\n", 0, ret_val);
        else {
            printf("Error for index %d (ret_val %d)\n", i, ret_val);
            return 0;
        }
    }

    printf("\nCreate %d tag services with normal keys (> 0)\n", test_tags);
    for(i = 1; i < test_tags; i++) {
        ret_val = tag_get(i, TAG_CREAT, TAG_PERM_ALL);
        if(ret_val == i + test_tags - 1)
            printf("Correct! tag_get (key %d) returned tag_descr %d\n", i, ret_val);
        else {
            printf("Error for index %d (ret_val %d)\n", i, ret_val);
            return 0;
        }
    }

    printf("\nOpen a Tag with IPC_PRIVATE Key\n");
    ret_val = tag_get(0, TAG_OPEN, TAG_PERM_ALL);
    if(ret_val == -1) printf("Correct! tag_get (key %d) could not be opened (ret = %d)\n", 0, ret_val);
    else { printf("Error! tag_get with IPC_PRIVATE returned a Tag descr (%d)\n", ret_val); return 0; }

    printf("\nCreate a new Tag with IPC_PRIVATE Key\n");
    ret_val = tag_get(0, TAG_CREAT, TAG_PERM_ALL);
    if(ret_val == test_tags * 2 - 1) printf("Correct! tag_get (key %d) created new tag (ret = %d)\n", 0, ret_val);
    else { printf("Error! tag_get with IPC_PRIVATE could not be created (ret_val = %d)\n", ret_val); return 0; }


    printf("\nOpening all previously created tag services\n");
    for(i = 1; i < test_tags; i++) {
        ret_val = tag_get(i, TAG_OPEN, TAG_PERM_ALL);
        if(ret_val == i + test_tags - 1)
            printf("Correct! tag_get (key %d) returned tag_descr %d\n", i, ret_val);
        else {
            printf("Error for index %d (ret_val %d)\n", i, ret_val);
            return 0;
        }
    }


    printf("\nDelete all instances\n");
    for(i = 0; i < test_tags * 2; i++) {
        ret_val = tag_ctl(i, TAG_DELETE);
        if(ret_val == 1)
            printf("Correct! tag_ctl (delete) (tag %d) deleted succesfully\n", i);
        else {
            printf("Error for index %d (ret_val %d)\n", i, ret_val);
            return 0;
        }
    }

    printf("\nTesting multiple 'tag_get' in create with IPC_PRIVATE Done\n\n", gettid());



    return 1;

}

int test_busy_ctl_interrupt() {
    
    int ret_val, ret;
    pthread_t recv_thread, ctl_thread;
    input_t input;

    printf("\nTesting a receive and ctl interaction (TID %d)\n\n", gettid());


    //PROVA A METTERE A RECEIVE THREAD BUFFER == NULL
    
    printf("\nTesting a Receive and Delete/Awake All \n");
    ret_val = tag_get(0, TAG_CREAT, TAG_PERM_USR);
    if(ret_val < 0) {
        printf("Error in creating Tag with IPC_PRIVATE (ret_val %d)\n", ret_val);
        return 0;
    }

    printf("Created Tag with descriptor %d\n", ret_val);

    input = (input_t){ .tag = ret_val, .level = 0, .size = 10};

    ret = pthread_create(&recv_thread, 0, receive_thread, &input);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    printf("Receive thread started\n");

    sleep(3);

    printf("Try to delete Tag while in use\n");

    ret = pthread_create(&ctl_thread, 0, delete_thread, &input);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }
    
    pthread_join(ctl_thread, 0);


    printf("Try to awake receiving thread\n");
    ret = pthread_create(&ctl_thread, 0, awake_thread, &input);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    printf("Awake All thread started\n");


    
    pthread_join(recv_thread, 0);
    pthread_join(ctl_thread, 0);

    printf("Receive thread woken up succesfully\n");
    

    printf("\nTesting an interrupt on a blocked receive\n");

    ret = pthread_create(&recv_thread, 0, receive_thread, &input);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    printf("Sending Interrupt Signal to stop receiving thread \n");
    
    sleep(2);
    
    pthread_kill(recv_thread, SIGINT);
    
    pthread_join(recv_thread, 0);

    printf("Receive thread woken up succesfully from interrupt\n");


    printf("\nDeleting tag\n");
    ret = pthread_create(&ctl_thread, 0, delete_thread, &input);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }
    
    pthread_join(ctl_thread, 0);


    printf("\nTesting a receive and ctl interaction Done\n\n");

}

void* receive_thread(void* input) {


    int tag, level, size, ret_val;
    char* buffer;

    tag   = ((input_t*) input) -> tag;
    level = ((input_t*) input) -> level;
    size  = ((input_t*) input) -> size;

    buffer = malloc(sizeof(char) * size);
    if(buffer == 0) printf("Error in allocating buffer for receiver (TID %d)\n", gettid());

    printf("[RECEIVE %d] Receive started\n", gettid());

    ret_val = tag_receive(tag, level, buffer, size);

    printf("[RECEIVE %d] Receive done. ret_val: %d, buffer: %s\n", gettid(), ret_val, buffer);

    return 0;

}
void* awake_thread(void* input) {

    int tag, ret_val;
    
    tag = ((input_t*) input) -> tag;
    
    printf("[AWAKE %d] Awake_all started\n", gettid());

    ret_val = tag_ctl(tag, TAG_AWAKE_ALL);

    printf("[AWAKE %d] Awake_all done. ret_val: %d\n", gettid(), ret_val);

    return 0;

}
void* delete_thread(void* input) {

    int tag, ret_val;
    
    tag = ((input_t*) input) -> tag;
    
    printf("[DELETE %d] Delete started\n", gettid());

    ret_val = tag_ctl(tag, TAG_DELETE);

    printf("[DELETE %d] Delete done. ret_val: %d\n", gettid(), ret_val);

    return 0;
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


