#!/bin/sh

# Find libraries mentioned in the User Guide and check for broken symbolic links in lib directories.

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

echo "    === Library exists test ===    "

echo "  --- find mentioned libaries ---  "

LIBRARIES="$LCG_LOCATION/lib/liblcg_util.so $LCG_LOCATION/lib/liblcg_util.a"
LIBRARIES="$LIBRARIES $LCG_LOCATION/lib/libgfal.so $LCG_LOCATION/lib/libgfal.a"
LIBRARIES="$LIBRARIES $GLOBUS_LOCATION/lib/libglobus_gass_copy_gcc32.so $GLOBUS_LOCATION/lib/libglobus_gass_copy_gcc32.a"

ERROR="no"

for name in $LIBRARIES
do

   if [ -r $name ]; then
     echo "$name - Ok"
   else
     echo "$name - NOT FOUND!"
     ERROR="yes"
   fi

done

echo "  --- search for broken links ---  "

for name in $LCG_LOCATION/lib/* $GLOBUS_LOCATION/lib/* $GLITE_LOCATION/lib/*
do
  if [ ! -r $name ]; then

    if [ -L $name ]; then
      echo "$name is a broken link"
    else
      echo "$name - file not readable"
    fi
    ERROR="yes"

  fi
done

if [ "$ERROR" == "yes" ]; then
  echo ' *** test NOT passed *** '
  exit 1
fi

echo " === test PASSED === "
exit 0
