#!/bin/bash

if [ x$(id -u) != x0 ]; then
    echo "Please run make install as root"
    exit 1
fi

if [ ! -f /etc/passwd ]; then
    echo "File /etc/shadow not found. Aborting."
    exit 2
fi

crypt_id=$(grep -m 1 -oP '(?<=\$)[1-6](?=\$)' /etc/shadow)
st=0
for entry in $(cat /etc/shadow|grep -oP '(?<=\$)[1-6](?=\$)'); do
    [ "$crypt_id" = "$entry" ]
    st=$(( $? + st ))
done

if [ ! $st -eq 0 ]; then
    echo "Different hashing algorithms are used in /etc/shadow. Aborting."
    exit 3
fi
echo "$crypt_id" > pald.conf
