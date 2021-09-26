#!/bin/bash

export TEST_SYSCALL=1
export MOD_DEBUG=1

root_dir = $PWD

cd syscall-table-disc

make clean
sudo dmesg -C
sudo rmmod SCTH
make all
sudo insmod SCTH.ko

cd ../test

make clean
ls -l
make test_syscall
./dummy_syscall.o

cd ..