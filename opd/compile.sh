#!/bin/bash
rm opd.out opd.o
cd reqsock
./compile.sh
cd ..

gcc -c opd.c
gcc -o opd.out opd.o reqsock/reqlib.o

