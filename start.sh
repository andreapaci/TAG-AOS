#!/bin/bash

# Enviroment variables
export TEST_FUNC=0          # set 1 to run functionality test on some utilities used in the modules, 0 otherwise
export TEST_SYSCALL=0       # set 1 to test the System Call Installer module without the Tag Module and run basic functionality test, 0 otherwise
export MOD_DEBUG=1          # set 1 to enable debug/extra printing on kernel-level log buffer, 0 otherwise


current_dir=${PWD##*/} 

if [[ $current_dir != "TAG-AOS" ]]
then
    printf "Error! Run script from 'TAG-AOS' folder\n"
    
else

# Clean the enviroment (unmounting modules and cleaning the compilation artifact)
bash clean.sh


if [[ $TEST_FUNC -eq 1 ]]
then
    printf "\n\nTesting utilities functionalities\n\n"
    
    cd ./test

    # Compile and run functionality test
    make test_func
    ./test_func.o
    
    make clean

else
    
    printf "\n\nMounting modules\n\n"
    
    # Show and clean kernel log buffer
    dmesg
    sudo dmesg -C

    cd syscall-table-disc

    # Compile and install System call table hacking module
    make all
    sudo insmod SCTH.ko

    
    if [[ $TEST_SYSCALL -eq 1 ]]
    then

        printf "\n\nTesting System call table hacker only\n\n"

        cd ../test

        # Test system call table hacking module

        make test_syscall
        ./dummy_syscall.o

        make clean

    else

        cd ../tag-module
        
        # Compile and install TAG module
        make all
        sudo insmod TAGMOD.ko

        cd ../test

        # Compile and run TAG module tester
        make test_tag_sys

        printf "\n\nNote: it will be now opened a new terminal (gnome-terminal as Ubuntu default) and it will\n"
        printf "      run a program which will read from the Char Device created along with the module\n"
        read -p "Press Enter to continue"

        sudo gnome-terminal -- './test_char_dev.o'

        printf "\n\n______________________________________________________\n"
        printf "Running TAG Module testing using only single CPU\n"
        printf "______________________________________________________\n\n"

        sudo taskset --cpu-list 1 ./test_tag.o
        printf "\n\n______________________________________________________\n"
        printf "End test single CPU\n"
        printf "______________________________________________________\n\n"

        printf "\n\n______________________________________________________\n"
        printf "Running TAG Module testing multiple CPUs\n"
        printf "______________________________________________________\n\n"

        sudo ./test_tag.o
        printf "\n\n______________________________________________________\n"
        printf "End test multiple CPUs\n"
        printf "______________________________________________________\n\n"
    fi

fi

cd ..

printf "\n\nDone\n\n"

fi