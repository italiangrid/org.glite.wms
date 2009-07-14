#!/bin/sh

id=`date +%s`
file=/tmp/env_${id}.txt
echo "$file"

#date > $file
#/usr/bin/id >> $file
env >> $file


