#!/bin/sh

# Analyse a list of commands and report those for which man pages can not be found.

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

echo "    === All-in-one manpage-exist test ===    "

NOT_FOUND=""

ntotal=0
nmissing=0;

for name in `awk -F '#' '{print $1}' commands.list`
do
   echo $name
   man -P "head | grep $name" $name

   if [ $? -ne 0 ]; then
     NOT_FOUND="$NOT_FOUND $name"
     ((nmissing=$nmissing+1))
   fi

   ((ntotal=$ntotal+1))
   echo ""
   
done

echo ""
echo " === Test results === "
echo "host: " `hostname --long`
echo "date: " `date`
echo "# man pages tested : $ntotal"
echo "# man pages missing: $nmissing"

if [ -z "$NOT_FOUND" ]; then
  echo "All man pages are present in the system"
  exit 0
fi

echo ""
echo " --- missing man pages --- "

for name in $NOT_FOUND
do
 echo $name
done

echo " --- end of list --- "
exit 1
