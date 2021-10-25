#!/bin/bash

printf "\n\nCleaning enviroment\n\n"

sudo rmmod TAGMOD
sudo rmmod SCTH

cd syscall-table-disc

make clean

cd ../tag-module

make clean

cd ../test

make clean

cd ..

printf "\n\nCleaning done.\n\n"