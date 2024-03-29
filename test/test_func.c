/**
 *  @file   test_funct.c
 *  @brief  Source code for testing the basic functionalities of the two struct used: Hashmap and Bitmask.
 *          This routine gets is independent from the modules developed
 *          It contains functionality tests on the Hashmap and Bitmask, containing also a basic performance measurement
 *              [NOTE] this routine is not involved in any manner in the project requirement: it has been developed only for "internal use"
 *              to check wether the structures work as intended and if no unexpected behaviour comes from using them,
 *              so the routine has not been developed with particular care regarding code style and shape.
 *              The tests contains a basic "assertion functionalities" to check if the returned value is coherent with
 *              what is expected to happen. If not, it exits the test with an error code (-1) 
 *  @author Andrea Paci
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <x86intrin.h>
#include <inttypes.h>
#include "../utils/include/bitmask.h"
#include "../utils/include/common.h"
#include "backup_hash/hashmap.h"


#define SEED0 401861
#define SEED1 879023
#define HASHMAP_CAP 2560

int test_bitmask(void);
int test_hashmap(void);


int main(int argc, void** argv) {

    printf("[TEST_FUNC] Testing\n");

    printf("\n\n[TEST_FUNC] Bitmask Testing\n");

    if(test_bitmask() == -1) return -1;


    printf("\n\n[TEST_FUNC] Hashmap Testing\n");

    if(test_hashmap() == -1) return -1;

    printf("\n\n[TEST_FUNC] All test executed correctly!\n");

}



int test_bitmask(void) {

    bitmask_t* mask;

    // Test basic bitmask initialization to check "borderline values" of bits on the bitmask
    // Note: 64 is a borderline value since the bitmask is made of "unisgned long long slots", so once
    //       the bitmask size reaches 65, it should use 2 slots instead of one  

    mask = initialize_bitmask(0);

    if(mask == 0) printf("[TEST_FUNC] Error initalizing mask with size 0, Correct!\n");
    else return -1;

    mask = initialize_bitmask(-1);

    if(mask == 0) printf("[TEST_FUNC] Error initalizing mask with size -1, Correct!\n");
    else return -1;

    mask = initialize_bitmask(1);
    if(mask == 0) return -1;
    
    printf("[TEST_FUNC] Mask with 1 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 1 && mask -> n_bits == 1) printf("[TEST_FUNC] Correct!\n");
    else{ printf("[TEST_FUNC] Wrong!\n"); return -1; }

    free_bitmask(mask);
    

    mask = initialize_bitmask(63);
    if(mask == 0) return -1;
    
    printf("[TEST_FUNC] Mask with 63 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 1 && mask -> n_bits == 63) printf("[TEST_FUNC] Correct!\n");
    else{ printf("[TEST_FUNC] Wrong!\n"); return -1; }
    
    free_bitmask(mask);


    mask = initialize_bitmask(64);
    if(mask == 0) return -1;
    
    printf("[TEST_FUNC] Mask with 64 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 1 && mask -> n_bits == 64) printf("[TEST_FUNC] Correct!\n");
    else{ printf("[TEST_FUNC] Wrong!\n"); return -1; }
    
    free_bitmask(mask);


    mask = initialize_bitmask(65);
    if(mask == 0) return -1;
    
    printf("[TEST_FUNC] Mask with 65 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 2 && mask -> n_bits == 65) printf("[TEST_FUNC] Correct!\n");
    else{ printf("[TEST_FUNC] Wrong!\n"); return -1; }
    
    free_bitmask(mask);





    


    // Testing several add/removal of bits in the bitmask, with the relative print of debug information
    // to check if the mask is updated as intended.



    printf("\n[TEST_FUNC] Getting new avaliable numbers\n");
    
    mask = initialize_bitmask(65);
    if(mask == 0) return -1;




    int number = get_avail_number(mask);
    if(number != 0){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu\n", number, *(mask -> mask));

    number = get_avail_number(mask);
    if(number != 1){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu\n", number, *(mask -> mask));

    number = get_avail_number(mask);
    if(number != 2){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu\n", number, *(mask -> mask));
    



    if(clear_number(mask, 1) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 1\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 1, Mask: %llu\n", *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 1){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu\n", number, *(mask -> mask));


    

    number = get_avail_number(mask);
    if(number != 3){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu\n", number, *(mask -> mask));

    number = get_avail_number(mask);
    if(number != 4){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu\n", number, *(mask -> mask));

    if(clear_number(mask, 2) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 2\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 2, Mask: %llu\n", *(mask -> mask));

    if(clear_number(mask, 3) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 2\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 3, Mask: %llu\n", *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 2){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu\n", number, *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 3){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu\n", number, *(mask -> mask));


    if(clear_number(mask, 4) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 4\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 4, Mask: %llu\n", *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 4){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu\n", number, *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 5){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu\n", number, *(mask -> mask));


    if(clear_number(mask, 4) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 4\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 4, Mask: %llu\n", *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 4){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu\n", number, *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 6){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu\n", number, *(mask -> mask));






    // Test clearing un-used bitmask number



    printf("\n[TEST_FUNC] Clearing numbers\n");

    if(clear_number(mask, 12) != 0) {
        printf("[TEST_FUNC] Error in clearing number: 12\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 12, Mask: %llu\n", *(mask -> mask));

    if(clear_number(mask, 63) != 0) {
        printf("[TEST_FUNC] Error in clearing number: 63\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 63, Mask: %llu\n", *(mask -> mask));

    if(clear_number(mask, 64) != 0) {
        printf("[TEST_FUNC] Error in clearing number: 64\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 64, Mask: %llu\n", *(mask -> mask));

    if(clear_number(mask, 65) != -1) {
        printf("[TEST_FUNC] Error in clearing number: 65\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 65, Mask: %llu\n", *(mask -> mask));

    if(clear_number(mask, 66) != -1) {
        printf("[TEST_FUNC] Error in clearing number: 66\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 66, Mask: %llu\n", *(mask -> mask));

    printf("[TEST_FUNC] Test Get/Clear free numbers executed correctly!\n");

    free_bitmask(mask);


    
    


    // Test maximum number retrival from the bitmask with a simple performance measurement
    
    
    printf("\n[TEST_FUNC] Test Maximum numbers allowed!\n");

    mask = initialize_bitmask(65);
    if(mask == 0) return -1;

    printf("[TEST_FUNC] Mask with 65 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 2 && mask -> n_bits == 65) printf("[TEST_FUNC] Correct!\n");
    else{ printf("[TEST_FUNC] Wrong!\n"); return -1; }

    for(int i = 0; i < 65; i++) {
        
        int number;
        unsigned long long cycle;

        cycle = rdtsc_fenced();

        number = get_avail_number(mask);

        cycle = rdtsc_fenced() - cycle;


        printf("\t Time to get free number = %llu\n", cycle);

        
        if(number != i){ 
            printf("[TEST_FUNC] Error in getting new number: %d\n", number);
            return -1;
        }
        printf("[TEST_FUNC] Number: %d, Mask: %llu, Mask#2: %llu\n", number, *((mask -> mask) + 1), *(mask -> mask));
    
    }

    // Trying to get a new number over the maximum allowed


    number = get_avail_number(mask);
    if(number != -1){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d Not allowed, Correct!, Mask: %llu, Mask#2: %llu\n", number, *((mask -> mask) + 1), *(mask -> mask));


    if(clear_number(mask, 54) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 54\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 54, Mask: %llu, Mask#2: %llu\n", *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 54){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu, Mask#2: %llu\n", number, *((mask -> mask) + 1), *(mask -> mask));

    free_bitmask(mask);






    // Test add/removal of bits with multiple slots involved



    printf("\n[TEST_FUNC] Test Multiple slots!\n");


    mask = initialize_bitmask(134);
    if(mask == 0) return -1;

    printf("[TEST_FUNC] Mask with 134 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 3 && mask -> n_bits == 134) printf("[TEST_FUNC] Correct!\n");
    else{ printf("[TEST_FUNC] Wrong!\n"); return -1; }

    for(int i = 0; i < 134; i++) {

        int number;
        unsigned long long cycle;

        cycle = rdtsc_fenced();

        number = get_avail_number(mask);

        cycle = rdtsc_fenced() - cycle;


        printf("\t Time to get free number = %llu\n", cycle);

        if(number != i){ 
            printf("[TEST_FUNC] Error in getting new number: %d\n", number);
            return -1;
        }
        printf("[TEST_FUNC] Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));
    
    }

    number = get_avail_number(mask);
    if(number != -1){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d Not allowed, Correct!, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));





    if(clear_number(mask, 54) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 54\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 54, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 54){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));




    if(clear_number(mask, 63) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 63\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 63, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 63){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));




    if(clear_number(mask, 64) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 64\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 64, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 64){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));




    if(clear_number(mask, 65) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 65\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 65, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 65){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));





    if(clear_number(mask, 67) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 67\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 67, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 67){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));





    if(clear_number(mask, 127) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 127\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 127, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 127){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));





    if(clear_number(mask, 128) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 128\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 128, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 128){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));






    if(clear_number(mask, 130) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 130\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 130, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 130){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));

    
    
    
    if(clear_number(mask, 133) != 1) {
        printf("[TEST_FUNC] Error in clearing number: 133\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 133, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));


    number = get_avail_number(mask);
    if(number != 133){ 
        printf("[TEST_FUNC] Error in getting new number: %d\n", number);
        return -1;
    }
    printf("[TEST_FUNC] Number: %d, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", number, *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));

    




    if(clear_number(mask, 134) != -1) {
        printf("[TEST_FUNC] Error in clearing number: 134\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 134, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));

    if(clear_number(mask, 135) != -1) {
        printf("[TEST_FUNC] Error in clearing number: 135\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 135, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));

    if(clear_number(mask, 344) != -1) {
        printf("[TEST_FUNC] Error in clearing number: 344\n");
        return -1;
    }
    printf("[TEST_FUNC] Cleared 344, Mask: %llu, Mask#2: %llu, Mask#3: %llu\n", *((mask -> mask) + 2), *((mask -> mask) + 1), *(mask -> mask));

    

    free_bitmask(mask);



   

    printf("[TEST_FUNC] Test Bitmask executed correctly!\n");
    return 0;

}




// Struct and custom function used for the Hashmap


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

    // Initialize hashmap which stores "data" object type

    struct hashmap* map = hashmap_new_with_allocator(
        malloc, 0, free, sizeof(data), 
        HASHMAP_CAP, SEED0, SEED1, 
        custom_hash, compare_hash, 0);
    if(map == 0) {
        printf("[TEST_FUNC] Hashamp non initalized!\n");
        return -1;
    }

    data data_ins = *((data *)malloc(sizeof(data)));
    data_ins.key = 1;
    data_ins.buffer = "chiave1";

    printf("[TEST_FUNC] Value: %d, %s\n", data_ins.key, data_ins.buffer);

    printf("[TEST_FUNC] Hashmap initalized\n");


    // Test some set/get/delete on the hashmap and see if the structures get's updated coherently

    hashmap_set(map, &data_ins);

    printf("[TEST_FUNC] Add 1 value\n");

    hashmap_set(map, &(data){ .key=2, .buffer="chiave2"});

    printf("[TEST_FUNC] Add 1 value\n");

    hashmap_set(map, &(data){ .key=3, .buffer="chiave3"});

    printf("[TEST_FUNC] Add 1 value\n");


    printf("[TEST_FUNC] Added %ld values\n", hashmap_count(map));

    if(hashmap_count(map) != 3) {
        printf("[TEST_FUNC] Number of entries should be 3!\n");
        return -1;
    }



    for(int key_ret = 1; key_ret < 4; key_ret++){
        data *data_ret = hashmap_get(map, &key_ret);
        if(data_ret == 0) {
             printf("[TEST_FUNC] Error getting data %d\n", key_ret);
             return -1;
        }
        printf("[TEST_FUNC]\tData %d: %d, %s\n", key_ret, data_ret -> key, data_ret -> buffer);

    }
    printf("[TEST_FUNC] 3 values added correctly\n");


    int key_ret = 4;
    data* data_ret = hashmap_get(map, &key_ret);
    if(data_ret != 0) {
        printf("[TEST_FUNC] Returned non-existing value: %d, %s\n", data_ret -> key, data_ret -> buffer);
        return -1;
    }

    printf("[TEST_FUNC] Non existing value not returned, correct\n");

    data_ret = hashmap_set(map, &(data){ .key=1, .buffer="nuovachiave"});
    if(data_ret == 0) {
         printf("[TEST_FUNC] No value replaced for key 1!\n");
         return -1;
    } 
    if(data_ret -> key != 1 || strcmp(data_ret -> buffer, "chiave1") != 0) {
        printf("[TEST_FUNC] Wrong value replaced for key 1: %d, %s\n", data_ret -> key, data_ret -> buffer);
        return -1;
    }

    printf("[TEST_FUNC] Old value replaced: %d, %s\n", data_ret -> key, data_ret -> buffer);


    key_ret = 1;
    data_ret = hashmap_get(map, &key_ret);
    if(data_ret == 0) {
         printf("[TEST_FUNC] No value returned for key 1!\n");
         return -1;
    } 
    if(data_ret -> key != 1 || strcmp(data_ret -> buffer, "nuovachiave") != 0) {
        printf("[TEST_FUNC] Wrong value returned: %d, %s\n", data_ret -> key, data_ret -> buffer);
        return -1;
    }
    printf("[TEST_FUNC] Value returned: %d, %s\n", data_ret -> key, data_ret -> buffer);

    key_ret = 4;
    data_ret = hashmap_delete(map, &key_ret);
    if(data_ret != 0) {
        printf("[TEST_FUNC] Deleted non-existing \n");
        return -1;
    }
    printf("[TEST_FUNC] Non-existing not deleted, Correct!\n");


    key_ret = 2;
    data_ret = hashmap_delete(map, &key_ret);
    if(data_ret == 0) {
         printf("[TEST_FUNC] No value returned for key 2!\n");
         return -1;
    } 
    if(data_ret -> key != 2 || strcmp(data_ret -> buffer, "chiave2") != 0) {
        printf("[TEST_FUNC] Wrong value deleted: %d, %s\n", data_ret -> key, data_ret -> buffer);
        return -1;
    }
    printf("[TEST_FUNC] Value deleted: %d, %s\n", data_ret -> key, data_ret -> buffer);

    printf("[TEST_FUNC] Testing 260 values\n");

    hashmap_set(map, &(data){ .key=2, .buffer="chiave2"});




    // Basic performance Test


    int tries, i;
    tries = 1556;
    i = 1;

    for(; i < tries; i++) {

        char *number = malloc(sizeof(char) * 20);
        sprintf(number,"chiave%d",i);

        unsigned long long cycle;
        
        cycle = rdtsc_fenced();
        cycle = rdtsc_fenced() - cycle;
        printf("\t Constant time = %llu\n", cycle);



        cycle = rdtsc_fenced();

        hashmap_set(map, &(data){ .key=i, .buffer=number});

        cycle = rdtsc_fenced() - cycle;


        printf("\t Time to set = %llu\n", cycle);


        cycle = rdtsc_fenced();

        data_ret = hashmap_get(map, &i);

        cycle = rdtsc_fenced() - cycle;


        printf("\t Time to get = %llu\n", cycle);

        printf("[TEST_FUNC] Value returned: %d, %s\n", data_ret -> key, data_ret -> buffer);
        
        if(data_ret -> key != i || strcmp(data_ret -> buffer, number) != 0) {
            printf("[TEST_FUNC] Wrong value returned\n");
            return -1;
        }
        printf("[TEST_FUNC] Correct %d\n", i);
    
    }
    
    printf("[TEST_FUNC] Freeing space\n");

    i = 1;
    for(; i < tries; i++) {

        data_ret = (data *) hashmap_delete(map, &i);
        if(data_ret == 0) { printf("[TEST_FUNC] Error in deleting element %d\n", i); return -1; }
        printf("[TEST_FUNC] Value deleted: %d, %s\n", data_ret -> key, data_ret -> buffer);
        free(data_ret -> buffer);
        
    }

    hashmap_free(map);    

    printf("[TEST_FUNC] Test Hashmap executed correctly!\n");

    return 0;

}