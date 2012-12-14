#!/bin/bash

function copy-path () { #copy something like /a/b/c/file to ./a/b/c/file
  # $1 = FULLPATH
  DIR=`echo "$1"|rev | cut -d \/ -f2-|rev`
  FILE=`echo "$1"|rev | cut -d \/ -f1|rev`

  mkdir -p "$HOME$DIR"
  cp "$1" "$HOME$1"
}


#copy all of progs into current directory
PROGS="/bin/sh /bin/ls"
HOME="$1"

for prog in $PROGS;  do
  copy-path "$prog" 
  PERM=`grep $FILE /srv/incperm/groupslist.conf|cut -d " " -f2`
  if [ ! -z "$PERM" ]
  then
    chown root:$PERM "$HOME$prog"
    chmod 770 "$HOME$prog"
  fi

  for lib in  $(ldd $prog | cut -d \( -f1 | cut -d \> -f2); do # copy all the libraries prog depends on
    copy-path $lib
  done
done                            

