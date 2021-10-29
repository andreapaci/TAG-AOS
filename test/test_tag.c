/**
 *  @file   test_tag.c
 *  @brief  Source code for testing the TAG Module, trying to test the behaviour of the module
 *          in both normal use and borderline scenarios. The test can be executed with the "taskset" in order
 *          to achive Single Core execution and verify that no deadlock occurs.
 *          There's a basic assertion mecanism that check if the system call ouput is coherent with the expected behaviour.
 *          In case something goes wrong, the test function exits and returns an error. Not every interaction with the module is asserted
 *          and so it must be checked using the output on the console.
 *          The test requires minimum interaction with the user. to further achieve debug information
 *          is possible to enable the DEBUG mode of the modules and have extra printed info using "dmesg"
 *          Those tests run using the advertised values:
 *              - Maximum number of Tags :  256
 *              - Number of levels       :   32
 *              - Tag Buffer size        : 4096
 *          So the correctness of the tests heavily depends on those values
 *  @author Andrea Paci
 */ 



#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <sys/syscall.h>
#include "tag.h"


#ifndef SYS_gettid
#error "SYS_gettid unavailable on this system"
#endif

#define SEPAR printf("-------------------------------------------------------------------\n"); 
#define gettid() ((pid_t)syscall(SYS_gettid))


// Structure used to pass argument to threads
typedef struct input_struct {
    int key;
    int tag;
    int level;
    size_t size;
    int iteration;
} input_t;


void* receive_thread(void* input);
void* send_thread(void* input);
void* awake_thread(void* input);
void* delete_thread(void* input);


int test_tag_get();
int test_ipc_private();
int test_busy_ctl_interrupt();
int test_basic_read_write(size_t read_size, size_t send_size);
int test_permissions();
int test_stress(int tags, int levels, int senders, int receivers, int iterations);


void interrupt_handler(int sig){
    printf("Signal received %d\n", sig);
}





// Main functions that starts all the tests
int main(int argc, char** argv) {
    
    signal(SIGINT, interrupt_handler);
    
    printf("Testing various Tag functionality.\n(Most of the debugging output will be visible with 'dmesg')\nPress Enter to continue...\n\n");
    getchar();

    SEPAR    
    printf("Test with tag_get() and tag_ctl() (delete). Press Enter to continue...\n");
    getchar();

    if(!test_tag_get()) return -1;

    printf("Test with tag_get() and tag_ctl() (delete) executed Succesfully!\n(check 'dmesg' for log if needed and possibly clear the log)\n\n");
    

    SEPAR
    printf("Test with tag_get() and IPC_PRIVATE. Press Enter to continue...\n");
    getchar();

    if(!test_ipc_private()) return -1;

    printf("Test with tag_get() and IPC_PRIVATE executed Succesfully!\n(check 'dmesg' for log if needed and possibly clear the log)\n\n");
    
    
    SEPAR
    printf("Test with tag_ctl()/Interrupt signal on occupied level. Press Enter to continue...\n");
    getchar();
    
    if(!test_busy_ctl_interrupt()) return -1;

    printf("Test with tag_ctl()/Interrupt signal on occupied level executed Succesfully!\n(check 'dmesg' for log if needed and possibly clear the log)\n\n");
    

    SEPAR
    printf("Test with basic send/receive. Press Enter to continue...\n");
    getchar();

    if(!test_basic_read_write(11, 11)) return -1;

    printf("Test with basic send/receive executed Succesfully!\n(check 'dmesg' for log if needed and possibly clear the log)\n\n");
    
    

    SEPAR
    printf("Test with different EUID! Press Enter to continue...");
    getchar();

    if(!test_permissions()) {
        
        seteuid(0);
        system("sudo userdel -f test_user1");
        system("sudo userdel -f test_user2");

        return -1;
    }
    printf("Test with different EUID executed Succesfully!\n(check 'dmesg' for log if needed and possibly clear the log)\n\n");
    
    

    SEPAR
    printf("Test with multiple send/receive. Use the other terminal to check on the state of the tags while the test runs.\nPress Enter to continue...\n");
    getchar();

    if(!test_stress(3, 4, 100, 100, 100)) return -1;

    printf("Test with multiple send/receive executed Succesfully!\n(check 'dmesg' for log if needed and possibly clear the log)\n\n");




}




// Test mulitple tag_get() and tag_ctl() in delete mode
int test_tag_get() {

    printf("\nTesting multiple 'tag_get' in create (TID %d)\n\n", gettid());
    
    int test_tags, max_tags, i, ret_val;
    test_tags = 300; 
    max_tags = 256;


    // Create more than allowed tag services and see how it reacts over the maximum threshold
    printf("\nCreating %d tag services\n", test_tags);
    for(i = 0; i < test_tags; i++) {
        ret_val = tag_get(((i + 1) * 3), TAG_CREAT, TAG_PERM_ALL);
        if(i < max_tags && ret_val == i)
            printf("Correct! tag_get (key %d) returned tag_descr %d\n", (i + 1) *3, ret_val);
        else if(i >= max_tags && ret_val < 0) 
            printf("Correct! Not possible to create more tags (max reached) (ret: %d)\n", ret_val);
        else {
            printf("Error for index %d (ret_val %d)\n", i, ret_val);
            return 0;
        }
    }


    // Open tag services and see how it reacts when tag doesn't exists
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


    // Delete random instances and see how the subsequent tag_get reacts   
    printf("\nDelete random instances (12, 23, 244)\n");
    
    ret_val = tag_ctl(23, TAG_DELETE);
    if(ret_val != 1) { printf("Error in deleting\n"); return 0; }

    ret_val = tag_ctl(244, TAG_DELETE);
    if(ret_val != 1) { printf("Error in deleting\n"); return 0; }

    ret_val = tag_ctl(12, TAG_DELETE);
    if(ret_val != 1) { printf("Error in deleting\n"); return 0; }


    
    // Try to delete an unexisting tag
    printf("\nDelete unexisting instance (12)\n");

    ret_val = tag_ctl(12, TAG_DELETE);
    if(ret_val != -1) { printf("Error. Unexisting tag found in deleting\n"); return 0; }


    // Create random instances expecting that the tag descriptors returned are 12, 23 and 244    
    printf("\nCreate random instances\n");
    
    ret_val = tag_get(23455, TAG_CREAT, TAG_PERM_ALL);
    if(ret_val != 12) { printf("Error in deleting\n"); return 0; }
    printf("Tag created: %d\n", ret_val);


    ret_val = tag_get(0, TAG_CREAT, TAG_PERM_ALL);
    if(ret_val != 23) { printf("Error in deleting\n"); return 0; }
    printf("Tag created: %d\n", ret_val);


    ret_val = tag_get(45100, TAG_CREAT, TAG_PERM_ALL);
    if(ret_val != 244) { printf("Error in deleting\n"); return 0; }
    printf("Tag created: %d\n", ret_val);
    
    printf("\nAll tag services created! Use the other terminal to see all the instaces on the device driver\n(or even $ sudo cat /dev/tag_info)\n");
    printf("Press Enter when done (it will delete all the instances and proceed on the next test)\n");
    getchar();

    // Delete all instances (even unexisting ones)
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






// Test mixed creation of IPC_PRIVATE and "normal" tags and check that no overlap occurs
int test_ipc_private() {
    int test_tags, i, ret_val;
    test_tags = 7; 
    
    printf("\nTesting multiple 'tag_get' in create with IPC_PRIVATE as key (TID %d)\n\n", gettid());

    // Create multiple IPC_PRIVATE Tags
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

    // Create multiple "normal" Tags and verify that the tag descriptor is acoounting of the previously
    // allocated IPC_PRIVATE Tags
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

    // Try a (failing) open of a IPC_PRIVATE Tag
    printf("\nOpen a Tag with IPC_PRIVATE Key\n");
    ret_val = tag_get(0, TAG_OPEN, TAG_PERM_ALL);
    if(ret_val == -1) printf("Correct! tag_get (key %d) could not be opened (ret = %d)\n", 0, ret_val);
    else { printf("Error! tag_get with IPC_PRIVATE returned a Tag descr (%d)\n", ret_val); return 0; }


    // Create a new IPC_PRIVATE Tag
    printf("\nCreate a new Tag with IPC_PRIVATE Key\n");
    ret_val = tag_get(0, TAG_CREAT, TAG_PERM_ALL);
    if(ret_val == test_tags * 2 - 1) printf("Correct! tag_get (key %d) created new tag (ret = %d)\n", 0, ret_val);
    else { printf("Error! tag_get with IPC_PRIVATE could not be created (ret_val = %d)\n", ret_val); return 0; }


    // Try to open Normal Tag services
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

    // Delete all instances (IPC_PRIVATE and normal)
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

    printf("\nTesting multiple 'tag_get' in create with IPC_PRIVATE Done\n\n");



    return 1;

}




// Test CTL while a level is occupied by a receive
// Test also a interrupt singal while receiver is in the waitqueue
int test_busy_ctl_interrupt() {
    
    int ret_val, ret;
    pthread_t recv_thread_1, recv_thread_2, recv_thread_3, recv_thread_4, ctl_thread;
    input_t input_1, input_2, input_3, input_4;

    printf("\nTesting a receive and ctl interaction (TID %d)\n\n", gettid());

    // Testing a delete while a receive thread is blocked in WQ
    printf("\nTesting a Receive and Delete/Awake All \n");



    // Create a Tag
    ret_val = tag_get(0, TAG_CREAT, TAG_PERM_USR);
    if(ret_val < 0) {
        printf("Error in creating Tag with IPC_PRIVATE (ret_val %d)\n", ret_val);
        return 0;
    }
    printf("Created Tag with descriptor %d\n", ret_val);

    input_1 = (input_t){ .tag = ret_val, .level = 0, .size = 10, .iteration = 1};
    input_2 = (input_t){ .tag = ret_val, .level = 24, .size = 10, .iteration = 1};
    input_3 = (input_t){ .tag = ret_val, .level = 17, .size = 10, .iteration = 1};
    input_4 = (input_t){ .tag = ret_val, .level = 32, .size = 10, .iteration = 1};

    // Spawn 4 receiving thread (one of them will file since it uses level 32 where levels
    // go from 0 to 31 included)
    ret = pthread_create(&recv_thread_1, 0, receive_thread, &input_1);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    ret = pthread_create(&recv_thread_2, 0, receive_thread, &input_2);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    ret = pthread_create(&recv_thread_3, 0, receive_thread, &input_3);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }
    // Failing one
    ret = pthread_create(&recv_thread_4, 0, receive_thread, &input_4);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }
    printf("Receive thread started\n");

    // Let the main thread sleep to ensure receiving threads are enlisted in WQ
    sleep(2);

    printf("Try to delete Tag while in use\n");
    //Delete will fail
    ret = pthread_create(&ctl_thread, 0, delete_thread, &input_1);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }
  
    pthread_join(ctl_thread, 0);

    


    // Test a Awake All on the same Tag services (all receiving threads should wakeup)
    printf("Try to awake receiving thread\n");
    ret = pthread_create(&ctl_thread, 0, awake_thread, &input_1);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    printf("Awake All thread started\n");


    
    pthread_join(recv_thread_1, 0);
    pthread_join(recv_thread_2, 0);
    pthread_join(recv_thread_3, 0);
    pthread_join(recv_thread_4, 0);
    pthread_join(ctl_thread, 0);

    printf("Receive thread woken up succesfully\n");
    



    // Test a interrupt signal on a receiving thread
    printf("\nTesting an interrupt on a blocked receive\n");

    ret = pthread_create(&recv_thread_1, 0, receive_thread, &input_1);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    ret = pthread_create(&recv_thread_2, 0, receive_thread, &input_2);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    ret = pthread_create(&recv_thread_3, 0, receive_thread, &input_3);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    ret = pthread_create(&recv_thread_4, 0, receive_thread, &input_4);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    printf("Sending Interrupt Signal to stop receiving thread \n");
    
    sleep(2);
    
    // Send SIGINT to threads (could also be done by $ kill -2 <tid>)
    pthread_kill(recv_thread_1, SIGINT);
    pthread_kill(recv_thread_2, SIGINT);
    pthread_kill(recv_thread_3, SIGINT);
    pthread_kill(recv_thread_4, SIGINT);
    
    
    pthread_join(recv_thread_1, 0);
    pthread_join(recv_thread_2, 0);
    pthread_join(recv_thread_3, 0);
    pthread_join(recv_thread_4, 0);

    printf("Receive thread woken up succesfully from interrupt\n");

    // Delete the tag
    printf("\nDeleting tag\n");
    ret = pthread_create(&ctl_thread, 0, delete_thread, &input_1);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }
    
    pthread_join(ctl_thread, 0);


    printf("\nTesting a receive and ctl interaction Done\n\n");

    return 1;

}




// Test a basic read/send interaction
// Different sizes are tried to check if they are correctly managed
int test_basic_read_write(size_t read_size, size_t send_size) {

    int ret_val, ret, tag;
    pthread_t recv_thread_1, recv_thread_2, snd_thread, snd_thread_2;
    input_t input_recv_1, input_recv_2, input_send;

    // Test basic send/receive
    printf("\nTesting a basic send/receive with sizes: receive: %ld, send %ld (TID %d)\n\n", read_size, send_size, gettid());

    // Create Tag
    printf("\nCreate Tag instance\n");
    tag = tag_get(0, TAG_CREAT, TAG_PERM_USR);
    if(tag < 0) {
        printf("Error in creating Tag with IPC_PRIVATE (tag %d)\n", tag);
        return 0;
    }
    printf("Created Tag with descriptor %d\n", tag);

    // Different receiving sizes are tried to check if the received message is actually different
    input_recv_1 = (input_t){ .tag = tag, .level = 31, .size = read_size - 1, .iteration = 1};
    input_recv_2 = (input_t){ .tag = tag, .level = 31, .size = read_size - 4, .iteration = 1};
    input_send = (input_t){ .tag = tag, .level = 31, .size = send_size, .iteration = 1};

    // Receiving thread started
    ret = pthread_create(&recv_thread_1, 0, receive_thread, &input_recv_1);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    ret = pthread_create(&recv_thread_2, 0, receive_thread, &input_recv_2);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    printf("Receive thread started\n");


    // Sleep to let the subsequent sender see both receiver in the level
    sleep(2);

    ret = pthread_create(&snd_thread, 0, send_thread, &input_send);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    printf("Send thread started\n");

    pthread_join(recv_thread_1, 0);
    pthread_join(recv_thread_2, 0);
    pthread_join(snd_thread, 0);


    // Same test as before, but this time the sender has the minor size amongst the 3
    // Moreover, another sender thread gets spawned to check that only one of the two will write
    // and the other will fail


    printf("\n\nSame test as before, but with sender using a smaller size (should see a smaller message than before)\n");

    input_recv_1 = (input_t){ .tag = tag, .level = 31, .size = read_size - 1, .iteration = 1};
    input_recv_2 = (input_t){ .tag = tag, .level = 31, .size = read_size - 4, .iteration = 1};
    input_send = (input_t){ .tag = tag, .level = 31, .size = send_size - 5, .iteration = 1};

    ret = pthread_create(&recv_thread_1, 0, receive_thread, &input_recv_1);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    ret = pthread_create(&recv_thread_2, 0, receive_thread, &input_recv_2);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    printf("Receive thread started\n");

    sleep(2);

    ret = pthread_create(&snd_thread, 0, send_thread, &input_send);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    ret = pthread_create(&snd_thread_2, 0, send_thread, &input_send);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }


    printf("Send thread started\n");

    pthread_join(recv_thread_1, 0);
    pthread_join(recv_thread_2, 0);
    pthread_join(snd_thread, 0);
    pthread_join(snd_thread_2, 0);


    printf("\nDone. Deleting tag\n");

    ret_val = tag_ctl(tag, TAG_DELETE);
    
    printf("Delete done. ret_val: %d\n", ret_val);

    return 1;
}



// Test different EUID for send/receive syscalls
// To make the test possible, two user are created
int test_permissions() {

    int ret_val, ret, tag;
    pthread_t recv_thread, snd_thread;
    input_t input_recv, input_send;

    printf("\nTesting a basic send/receive with different EUID (TID %d)\n\n", gettid());

    printf("Note! This will create two user (test_user1 and test_user2) at UID 3000 and 3001.\nPress Enter to continue...\n");
    getchar();

    // Create two user
    system("sudo adduser --uid 3000 --disabled-password --no-create-home --gecos GECOS test_user1");
    system("sudo adduser --uid 3001 --disabled-password --no-create-home --gecos GECOS test_user2");

    


    // Set to a random EUID
    if(seteuid(3000) == -1) { printf("Error setting EUID (%d) \n", errno); return 0; }

    // Create Tag service
    printf("\nCreate Tag instance\n");
    tag = tag_get(1337, TAG_CREAT, TAG_PERM_USR);
    if(tag < 0) {
        printf("Error in creating Tag with IPC_PRIVATE (tag %d)\n", tag);
        return 0;
    }
    printf("Created Tag with descriptor %d\n", tag);

    input_recv = (input_t){ .tag = tag, .level = 18, .size = 10, .iteration = 1};
    input_send = (input_t){ .tag = tag, .level = 18, .size = 5, .iteration = 1};



    // Start a Send and a receive with wrong EUID
    printf("\nTesting send/receive with Wrong EUID\n");

    if(seteuid(0) == -1) { printf("Error setting EUID (%d) \n", errno); return 0; }
    if(seteuid(3001) == -1) {  printf("Error setting EUID (%d) \n", errno); return 0; }

    ret = pthread_create(&recv_thread, 0, receive_thread, &input_recv);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    printf("Receive thread started (Wrong EUID, should fail)\n");

    sleep(2);

    ret = pthread_create(&snd_thread, 0, send_thread, &input_send);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    printf("Send thread started (Wrong EUID, should fail)\n");

    pthread_join(recv_thread, 0);
    pthread_join(recv_thread, 0);
    pthread_join(snd_thread, 0);




    // Start a Send and a receive with a correct EUID
    printf("\nTesting send/receive with Correct EUID\n");


    if(seteuid(0) == -1) { printf("Error setting EUID (%d) \n", errno); return 0; }
    if(seteuid(3000) == -1) {  printf("Error setting EUID (%d) \n", errno); return 0; }

    ret = pthread_create(&recv_thread, 0, receive_thread, &input_recv);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    printf("Receive thread started (Correct EUID)\n");

    sleep(2);

    ret = pthread_create(&snd_thread, 0, send_thread, &input_send);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    printf("Send thread started (Correct EUID)\n");

    pthread_join(recv_thread, 0);
    pthread_join(recv_thread, 0);
    pthread_join(snd_thread, 0);


    // Start a Send and a receive with Root EUID
    printf("\nTesting send/receive with Root EUID\n");


    if(seteuid(0) == -1) { printf("Error setting EUID (%d) \n", errno); return 0; }

    ret = pthread_create(&recv_thread, 0, receive_thread, &input_recv);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    printf("Receive thread started (Correct EUID)\n");

    sleep(2);

    ret = pthread_create(&snd_thread, 0, send_thread, &input_send);
    if(ret != 0) {
        printf("Error creating thread, error: %d\n", ret);
        return 0;
    }

    printf("Send thread started (Correct EUID)\n");

    pthread_join(recv_thread, 0);
    pthread_join(recv_thread, 0);
    pthread_join(snd_thread, 0);


    // Delete created user
    system("sudo userdel -f test_user1");
    system("sudo userdel -f test_user2");

    printf("\nDone. Deleting tag\n");

    ret_val = tag_ctl(tag, TAG_DELETE);

    printf("Delete done. ret_val: %d\n", ret_val);

    printf("\n\nTesting send/receive with Various EUID Done\n\n");


    return 1;
}


// Stress test for Tag Service. For each tag created (sepcified in "tags") it creates "levels" number of lever, and
// for each of them, it spawns "senders" sender threads and "receivers" receiver threads, which each one iterate the send/recive
// for "iterations" number of time
int test_stress(int tags, int levels, int senders, int receivers, int iterations){

    
    printf("\nTesting a stress test with send/receive (TID %d)\n\n", gettid());

    int tag, i, j, k;

    int ret_val, ret;
    pthread_t recv_thread[tags * levels * receivers], snd_thread[tags * levels * senders];
    input_t *input_recv[tags * levels * receivers], *input_send[tags * levels * senders];

    // For each tag
    for(i = 0; i < tags; i++) {

        // Create Tag
        printf("\nCreate Tag instance %d\n", i);
        tag = tag_get(0, TAG_CREAT, TAG_PERM_USR);
        if(tag < 0) {
            printf("Error in creating Tag with IPC_PRIVATE (tag %d)\n", tag);
            return 0;
        }
        printf("Created Tag with descriptor %d\n", tag);

        // For each level
        for(j = 0; j < levels; j++) {

            // Foe each receiver...
            for(k = 0; k < receivers; k++) {

                int tag_val, level_val;
                
                tag_val = i;
                level_val = j;

                input_recv[i * (levels * receivers) + j * receivers + k] = malloc(sizeof(input_t));
                if(input_recv[i * (levels * receivers) + j * receivers + k] == 0) { printf("Error in allocating the input\n"); return 0; }

                input_recv[i * (levels * receivers) + j * receivers + k] -> tag = tag_val;
                input_recv[i * (levels * receivers) + j * receivers + k] -> level = level_val;
                input_recv[i * (levels * receivers) + j * receivers + k] -> size = 64;
                input_recv[i * (levels * receivers) + j * receivers + k] -> iteration = iterations;
    

                // Start a receiving thread
                ret = pthread_create(&(recv_thread[i * (levels * receivers) + j * receivers + k]), 0, receive_thread, (void *) input_recv[i * (levels * receivers) + j * receivers + k]);
                if(ret != 0) {
                    printf("Error creating thread, error: %d\n", ret);
                    return 0;
                }
                printf("Receive thread started (Tag : %d, Level: %d, Number %d)\n", tag, j, k);
            }

            printf("Wait 5 seconds to start sender thread at Tag %d and Level %d\n", i, j);
            sleep(1);

            // ... and for each sender
            for(k = 0; k < senders; k++) {

                int tag_val, level_val;
                
                tag_val = i;
                level_val = j;

                input_send[i * (levels * senders) + j * senders + k] = malloc(sizeof(input_t));
                if(input_send[i * (levels * senders) + j * senders + k] == 0) { printf("Error in allocating the input\n"); return 0; }
                input_send[i * (levels * senders) + j * senders + k] -> tag = tag_val;
                input_send[i * (levels * senders) + j * senders + k] -> level = level_val;
                input_send[i * (levels * senders) + j * senders + k] -> size = 64;
                input_send[i * (levels * senders) + j * senders + k] -> iteration = iterations;


                // Start a sender thread
                ret = pthread_create(&(snd_thread[i * (levels * senders) + j * senders + k]), 0, send_thread, (void *) input_send[i * (levels * senders) + j * senders + k]);
                if(ret != 0) {
                    printf("Error creating thread, error: %d\n", ret);
                    return 0;
                }
                printf("Sending thread started (Tag : %d, Level: %d, Number %d)\n", tag, j, k);
            }


        }


    }

    printf("All thread started!\n");

    sleep(5);
    
    // For each tag
    for(i = 0; i < tags; i++) {
        // For each level
        for(j = 0; j < levels; j++) {
            // ... and for each sender
            for(k = 0; k < senders; k++) {
                //Wait for each thread
                pthread_join((snd_thread[i * (levels * senders) + j * senders + k]), 0);
                free((input_send[i * (levels * senders) + j * senders + k]));
            }

            printf("All senders at Tag %d, level %d Terminated!\n", i, j);

        }
        printf("All senders at Tag %d, Terminated!\n", i);
        printf("Sending CTL AWAKE_ALL in 5 seconds... (tag: %d)\n", i);
        sleep(5);
        printf("Awake all at Tag %d!\n", i);

        // Once every sender finished, AWAKE_ALL the receiving threads
        ret_val = tag_ctl(i, TAG_AWAKE_ALL);
        if(ret_val < 0) {
            printf("Error in Awaking All thread from Tag %d and level %d\n", i, j);
            return 0;
        }

        printf("Tag Awake ALL sent at Tag %d (ret_val = %d)\n", i, ret_val);

        // For each level
        for(j = 0; j < levels; j++) {

            //Wait for all receiving thread
            
            // For each Receiver
            for(k = 0; k < receivers; k++) {
                //Wait for each thread 
                pthread_join((recv_thread[i * (levels * receivers) + j * receivers + k]), 0);
                free((input_recv[i * (levels * receivers) + j * receivers + k]));
            }

            printf("All threads from Tag %d and level %d exited.\n", i, j);

        
        }
    }
    
    printf("All threads returned\n");
    printf("Before deleting, it's possible to check the other terminal to check on the tags\nPress Enter to clean the tags...\n");
    getchar();
    


    // Delete all tags
    // For each tag
    for(i = 0; i < tags; i++) {
        ret_val = tag_ctl(i, TAG_DELETE);
        if(ret_val <= 0) {
            printf("Error in Deleting Tag %d and level %d (Probably occupied by the read on Char Device)\n", i, j);
        }
    }


    printf("\nStress test Done!\n");

    return 1;
}











// Code for threads


void* receive_thread(void* input) {


    int tag, level, size, ret_val, iteration, i;
    char* buffer;
    
    tag         = ((input_t*) input) -> tag;
    level       = ((input_t*) input) -> level;
    size        = ((input_t*) input) -> size;
    iteration   = ((input_t*) input) -> iteration;

    buffer = malloc(sizeof(char) * (size + 1));
    if(buffer == 0) printf("Error in allocating buffer for receiver (TID %d)\n", gettid());

    printf("[RECEIVE %d] Receive started\n", gettid());

    for(i = 0; i < iteration; i++) {
        memset(buffer, 0, sizeof(char) * (size + 1));
        ret_val = tag_receive(tag, level, buffer, size);
        printf("[RECEIVE %d] Receive done. ret_val: %d, buffer: %s, Last char: %d\n", gettid(), ret_val, buffer, buffer[size]);
        if(ret_val == 0) { printf("[RECEIVE %d] Receive got it by Interrupt/Awake All. Exiting.\n", gettid()); break; }
        if(ret_val < 0)  { printf("[RECEIVE %d] Error in receiving. Exiting\n", gettid()); break; }
    }

    printf("[RECEIVE %d] Receive exiting\n", gettid());
    free(buffer);

    return 0;

}

void* send_thread(void* input) {


    int tag, level, size, ret_val, iteration, i;
    char* buffer;

    tag         = ((input_t*) input) -> tag;
    level       = ((input_t*) input) -> level;
    size        = ((input_t*) input) -> size;
    iteration   = ((input_t*) input) -> iteration;

    buffer = malloc(sizeof(char) * 65);
    if(buffer == 0) printf("Error in allocating buffer for sender (TID %d)\n", gettid());
   

    printf("[SEND %d] Sender started\n", gettid());

    for(i = 0; i < iteration; i++) {
        memset(buffer, 0, sizeof(char) * 65);
        printf("[SEND %d] Sender calling (iteration %d)\n", gettid(), i);
        if(sprintf(buffer, "Messaggio-prova(Tag %d, Level %d, Iteration %d)", tag, level, i) < 0) {
            printf("[SEND %d] Error in generating message buffer-\n", gettid());
            break;
        }
        ret_val = tag_send(tag, level, buffer, size);
        printf("[SEND %d] Sender done. ret_val: %d\n", gettid(), ret_val);
        if(ret_val == 0) { printf("[SEND %d] Send Occupied/No Receiver.\n", gettid());}
        if(ret_val < 0)  { printf("[SEND %d] Error in sending. Exiting\n", gettid()); break; }
    
    }

    printf("[SEND %d] Send exiting\n", gettid());
    free(buffer);
    

    return 0;

}

void* awake_thread(void* input) {

    int tag, ret_val, iteration, i;
    
    tag         = ((input_t*) input) -> tag;
    iteration   = ((input_t*) input) -> iteration;
    
    printf("[AWAKE %d] Awake_all started\n", gettid());

    for(i = 0; i < iteration; i++) {
        ret_val = tag_ctl(tag, TAG_AWAKE_ALL);
        printf("[AWAKE %d] Awake_all done. ret_val: %d\n", gettid(), ret_val);
    }

    return 0;

}

void* delete_thread(void* input) {

    int tag, ret_val, iteration, i;
    
    tag         = ((input_t*) input) -> tag;
    iteration   = ((input_t*) input) -> iteration;
    
    printf("[DELETE %d] Delete started\n", gettid());

    for(i = 0; i < iteration; i++) {
        ret_val = tag_ctl(tag, TAG_DELETE);
        printf("[DELETE %d] Delete done. ret_val: %d\n", gettid(), ret_val);
    }

    return 0;
}


