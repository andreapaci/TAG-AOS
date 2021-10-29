
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

extern int errno;
static FILE* file;
char* buffer;

void interrupt_handler(int sig){
    printf("Signal received %d\n", sig);
    if(buffer != 0) free(buffer);
    exit(fclose(file));
}

int main(int argc, char* argv[]) {

    size_t size; 
    

    signal(SIGINT, interrupt_handler);

    file = fopen("/dev/tag_info", "r");
    if(file == 0) {
        printf("Error opening file: %d\n", errno);
        return -1;
    }

    
    size = 10 * 4096;

    buffer = malloc(sizeof(char) * (size + 1));
    if(buffer == 0) {
        printf("Error allocing memory for buffer\n");
        return -1;
    }
    memset(buffer, 0, sizeof(char) * (size + 1));

    printf("Reading from file /dev/tag_info\n");

    while(1) {

        ssize_t char_read;

        printf("\nPress Enter to start reading from the Char Device or Ctrl + C to End\n");
        getchar();

        char_read = 0;

        rewind(file);

        do {

            char_read = fread(buffer, sizeof(char), size, file);
            
            if(char_read < 0) {
                printf("Error in reading from file\n");
                if(buffer != 0) free(buffer);
                exit(fclose(file));
            } 
            printf("%s", buffer);

            memset(buffer, 0, sizeof(char) * (size + 1));

        }while(char_read != 0);        

    }


}