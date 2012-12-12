#!/bin/bash
rm sampleclient.o reqlib.o client.out
gcc -c sampleclient.c
gcc -c reqlib.c

gcc -o client.out sampleclient.o reqlib.o

chgrp 11 client.out
chmod 2777 client.out
