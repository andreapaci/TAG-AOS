#include <unistd.h>
#include <stdio.h>

/**
 *  @file   dummy_syscall.c
 *  @brief  Source code for testing the dummy system call installed. The dummy system call
 * 			does a basic functionality test on the two dedicated structures used to support
 * 			the tag service: the Hashmap and the Bitmask. The source code of the test is in the file
 * 			"syscall-table-disc/scth_module.c" as one of the last defined routines
 *  @author Andrea Paci
 */ 
int main(int argc, char** argv){
	printf("The dummy syscall installed will be called\n");
	syscall(134,1,2);
	printf("The dummy syscall done\nTo check the output use 'dmesg'\n");
}