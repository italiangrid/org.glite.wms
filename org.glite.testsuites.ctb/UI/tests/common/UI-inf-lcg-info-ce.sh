#!/bin/sh

# Run lcg-info with --list-ce.
# Usage: UI-inf-lcg-info-ce.sh [--vo <vo>]
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

function exit_failure() {

  echo ""
  echo " *** lcg-info failed *** "
  echo " *** test NOT passed *** "
  exit 1
}

echo ""
echo "    === lcg-info CE test ===    "
echo ""

# ... get vo option if specified

if [ "$1" == "--vo" ] && [ -n "$2" ]; then
  echo " # lcg-info test # VO chosen: $2"
  VO_OPTIONS="--vo $2"
  shift
  shift
else
  echo " # lcg-info test # no VO chosen"
  VO_OPTIONS=""
fi

# ... ask lcg-info for a selection of "interesting" CE attributes

echo " # lcg-info test # listing software tags"
echo ""

lcg-info $VO_OPTIONS --list-ce --attr "Tag" || exit_failure

echo " # lcg-info test # listing selected CE attributes"
echo ""

lcg-info $VO_OPTIONS --list-ce --attr "OS,OSVersion,OSRelease,Processor,TotalCPUs,FreeCPUs,CEVOs" || exit_failure

if [ -z "$VO_OPTIONS" ]; then
  echo " # lcg-info test # You can supply VO with --vo to reduce the output"
  echo ""
fi


echo " # lcg-info test # testing command behaviour with nonexisting server"
echo ""
LCG_GFAL_INFOSYS=no.such.machine.cern.ch lcg-info $VO_OPTIONS --list-ce -attr "Tag"

if [ $? -eq 0 ] ; then
  echo " # lcg-info test # ERROR: The lcg-info command does not give an error when a nonexisting server is used" 
  exit_failure
fi

echo " # lcg-info test # lcg-info gives an error when a nonexisting server is used, as it should"
echo ""


echo "    === test PASSED === "
exit 0;
