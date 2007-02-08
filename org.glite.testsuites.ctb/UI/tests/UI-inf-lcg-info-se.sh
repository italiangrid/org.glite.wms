#!/bin/bash

# Run lcg-info with --list-se.
# Usage: UI-inf-lcg-info-se.sh --vo <vo>
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

echo ""
echo "    === lcg-info SE test ===    "
echo ""

# ... get vo option if specified

if [ "$1" == "--vo" ] && [ -n "$2" ]; then
  echo " # lcg-info test # VO chosen: $2"
  VO_OPTIONS="--vo $2"
  shift
  shift
else
  echo " # lcg-info test # Warning: no VO chosen - will use dteam"
  VO_OPTIONS="--vo dteam"
fi

# ... ask lcg-info for a selection of "interesting" SE attributes

echo " # lcg-info test # listing selected SE attributes"
echo ""

lcg-info $VO_OPTIONS --list-se --attr "SEName,SEArch,SEVOs,Path,Accesspoint,Protocol,UsedSpace,AvailableSpace" 

if [ $? -ne 0 ]; then

  echo ""
  echo " *** lcg-info failed *** "
  echo " *** test NOT passed *** "
  exit 1

fi

echo "    === test PASSED === "
exit 0;
