#!/bin/sh

LINKS=`stat -c %h $HOME` || LINKS=""
DIR_SIZE=`stat -c %s $HOME` || DIR_SIZE=""
BLOCK_SIZE=`stat -c %o $HOME` || BLOCK_SIZE=""

if [ ! "$HOME" ]; then
    echo "summary: \$HOME not set"
    exit 1
elif [ "$LINKS" -gt 10 ]; then
    echo "summary: too many subdirs in $HOME: $LINKS"
    exit 2
elif [ "$DIR_SIZE" -gt "$BLOCK_SIZE" ]; then
    echo "summary: too many files in $HOME: size=$DIR_SIZE blocksize=$BLOCK_SIZE"
    exit 2
else
    echo "summary: $HOME has ${DIR_SIZE}kB and $LINKS links"
    echo "-TEST PASSED-"
    exit 0
fi

