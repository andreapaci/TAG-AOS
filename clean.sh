#!/bin/bash

echo "Cleaning enviroment"

sudo rmmod TAGMOD
sudo rmmod SCTH

cd syscall-table-disc

make clean

cd ../tag-module

make clean

cd ../test

make clean

cd ..