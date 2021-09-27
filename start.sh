#!/bin/bash

export TEST_SYSCALL=0
export MOD_DEBUG=1

root_dir = $PWD

sh clean.sh

dmesg

sudo dmesg -C

cd syscall-table-disc

make all
sudo insmod SCTH.ko

cd ../tag-module

make all
sudo insmod TAGMOD.ko

cd ../test

#make test_syscall
#./dummy_syscall.o
make test_tag_sys
./test_tag.o

cd ..