#!/bin/sh

time=0
while true ; do
    prima=`date +%s`
    eval "($*)"
    dopo=`date +%s`
    diff=$(($dopo - $prima))

    if [ $diff -gt 60 ] ; then
	time=-1;
    fi
    if [ $diff -gt 30 ] ; then  
       time=$((5 + $time))
    else
       time=$((1 + $time))
    fi
    sleep $time
done
