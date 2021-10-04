#include <stdio.h>
#include "../tag-module/util/bitmask.h"


int main(int argc, void** argv) {

    printf("[TEST_FUNC] Testing\n");

    bitmask* mask = initialize_bitmask(0);

    if(mask == 0) printf("[TEST_FUNC] Error initalizing mask with size 0, Correct!\n");
    else return -1;

    mask = initialize_bitmask(-1);

    if(mask == 0) printf("[TEST_FUNC] Error initalizing mask with size -1, Correct!\n");
    else return -1;

    // NOTE: Automate this TEST FROM HERE --------------------------------

    mask = initialize_bitmask(1);
    if(mask == 0) return -1;
    
    printf("[TEST_FUNC] Mask with 1 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 1 && mask -> n_bits == 1) printf("[TEST_FUNC] Correct!\n");
    else{ printf("[TEST_FUNC] Wrong!\n"); return -1; }

    
    clear_bitmask(mask);
    


    mask = initialize_bitmask(63);
    if(mask == 0) return -1;
    
    printf("[TEST_FUNC] Mask with 63 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 1 && mask -> n_bits == 63) printf("[TEST_FUNC] Correct!\n");
    else{ printf("[TEST_FUNC] Wrong!\n"); return -1; }
    
    clear_bitmask(mask);




    mask = initialize_bitmask(64);
    if(mask == 0) return -1;
    
    printf("[TEST_FUNC] Mask with 64 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 1 && mask -> n_bits == 64) printf("[TEST_FUNC] Correct!\n");
    else{ printf("[TEST_FUNC] Wrong!\n"); return -1; }
    
    clear_bitmask(mask);



    mask = initialize_bitmask(65);
    if(mask == 0) return -1;
    
    printf("[TEST_FUNC] Mask with 65 bit initialized\n\tNumber of slots: %d Number of bits: %d\n", 
                mask -> slots, mask -> n_bits);
    if(mask -> slots == 2 && mask -> n_bits == 65) printf("[TEST_FUNC] Correct!\n");
    else{ printf("[TEST_FUNC] Wrong!\n"); return -1; }
    
    clear_bitmask(mask);

    // TO HERE ------------------------------------------------------------









    printf("[TEST_FUNC] Getting new avaliable numbers\n");
    
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







    printf("[TEST_FUNC] Clearing numbers\n");

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

    clear_bitmask(mask);






    printf("[TEST_FUNC] Test executed correctly!\n");
    return 0;

}