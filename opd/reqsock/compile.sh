#!/bin/bash
rm sampleclient.o reqlib.o client.out
gcc -c sampleclient.c
gcc -c reqlib.c

gcc -o client.out sampleclient.o reqlib.o
