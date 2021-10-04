#!/bin/bash

export TEST_FUNC=1
export TEST_SYSCALL=0
export MOD_DEBUG=1

#root_dir = $PWD

sh clean.sh

if [[ $TEST_FUNC -eq 1 ]]
then
    printf "\n******************\nTesting utilities functionalities\n\n"
    
    cd ./test
    make test_func
    ./test_func.o
    make clean

else
    
    printf "\n******************\nTesting modules\n\n"
    dmesg

    sudo dmesg -C

    cd syscall-table-disc

    make all
    sudo insmod SCTH.ko

    cd ../tag-module

    make all
    sudo insmod TAGMOD.ko

    cd ../test

    if [[ $TEST_SYSCALL -eq 1 ]]
    then
        make test_syscall
        ./dummy_syscall.o
    else
        make test_tag_sys
        ./test_tag.o
    fi

fi

cd ..

printf "\n******************\nDone\n\n"