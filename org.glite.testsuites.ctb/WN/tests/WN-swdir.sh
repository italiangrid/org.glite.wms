#!/bin/sh


SAME_OK=0
SAME_ERROR=1

. lib.sh

# get VO software dir
VO=`vo`
if [ -z "$VO" ]; then
    exit_with_summary $SAME_WARNING "could not determine VO"
fi
VO=`echo $VO | tr '[a-z].-' '[A-Z]__'`
SW_DIR=`eval echo \\\$VO_${VO}_SW_DIR`

# check VO software dir
if [ -z "$SW_DIR" ]; then
    exit_with_summary $SAME_ERROR "VO_${VO}_SW_DIR not set"
elif [ ! -d "$SW_DIR" ]; then
    exit_with_summary $SAME_ERROR "$SW_DIR not a directory"
elif ! [ -r "$SW_DIR" -a -x "$SW_DIR" ]; then
    exit_with_summary $SAME_ERROR "$SW_DIR not accessible"
elif [ "`stat -c %U $SW_DIR`" == "root" ]; then
    exit_with_summary $SAME_WARNING "$SW_DIR owned by root"
else # all good
    exit_with_summary $SAME_OK `ls -ld $SW_DIR/`
fi

