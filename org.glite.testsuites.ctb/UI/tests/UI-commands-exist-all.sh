#!/bin/sh

# Analyse a list of commands and report those missing in the system.

# Note that "missing in the system" here means that the command could not be found
# in the search path at the time the test was running.
# In principle the search path seen by the test could differ from that of the parent
# shell as well as other shell invocations.
# The simple sh shell is chosen for this script in order to minimize that effect.
# However keep in mind that shell environment may need extra attention.

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

echo "    === All-in-one commands-exist test ===    "

NOT_FOUND=""

ntotal=0
nmissing=0;

for name in `awk -F '#' '{print $1}' commands.list`
do
   /usr/bin/which $name

   if [ $? -ne 0 ]; then
     NOT_FOUND="$NOT_FOUND $name"
     ((nmissing=$nmissing+1))
   fi

   ((ntotal=$ntotal+1))

   # man -P "head | grep $name" $name || NOT_FOUND="$NOT_FOUND $name"
done

echo ""
echo " === Test results === "
echo "host: " `hostname --long`
echo "date: " `date`
echo "# commands tested : $ntotal"
echo "# commands missing: $nmissing"

if [ -z "$NOT_FOUND" ]; then
  echo "All commands are present in the system"
  exit 0
fi

echo ""
echo " --- missing commands --- "

for name in $NOT_FOUND
do
 echo $name
done

echo " --- end of list --- "
exit 1
