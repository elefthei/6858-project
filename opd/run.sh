#!/bin/bash
rm -f op.out client.out
gcc op.c -o op.out
gcc client.c -o client.out
./op.out
