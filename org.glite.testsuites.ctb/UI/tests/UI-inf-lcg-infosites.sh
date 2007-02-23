#!/bin/sh

# Runs lcg-infosites with various options and report failures if any.
# Usage: UI-inf-lcg-infosites.sh [--vo <vo>]
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

# ... Special echo
function myecho() {

  echo ""
  echo "    --- $@ --- "
  echo ""
}

# ... "Bad" exit
function exit_failure() {

  echo ""
  echo " *** lcg-infosites failed *** "
  echo " *** test NOT passed *** "
  exit 1
}


echo ""
echo "    === lcg-infosites test ===    "

# ... get vo option if specified

if [ "$1" == "--vo" ] && [ -n "$2" ]; then
  VO=$2
  shift
  shift
  echo "VO chosen: $VO"
else
  echo " # lcg-info test # Warning: no VO chosen - will use dteam"
  VO=dteam
fi


# ... make sure LCG_GFAL_INFOSYS is defined

echo " # lcg-info test # LCG_GFAL_INFOSYS=$LCG_GFAL_INFOSYS"

if [ -z "$LCG_GFAL_INFOSYS" ]; then
   myecho "ERROR: LCG_GFAL_INFOSYS is not defined - lcg-infosites may not work !"
   exit_failure
fi


# ... launch lcg-infosites with various options

myecho "site names"
lcg-infosites --vo $VO sitenames || exit_failure

myecho "CE"
lcg-infosites --vo $VO ce || exit_failure

myecho "CE with -v 2"
lcg-infosites --vo $VO ce -v 2 || exit_failure

myecho "SE"
lcg-infosites --vo $VO se || exit_failure

myecho "close SE"
lcg-infosites --vo $VO closeSE || exit_failure

myecho "software tags"
lcg-infosites --vo $VO tag || exit_failure

myecho "LFC catalog location"
lcg-infosites --vo $VO lfc || exit_failure

myecho "local LFC catalog location"
lcg-infosites --vo $VO lfcLocal || exit_failure

myecho "RB hostnames"
lcg-infosites --vo $VO rb || exit_failure

myecho "Data Location Index servers"
lcg-infosites --vo $VO dli || exit_failure

myecho "local Data Location Index servers"
lcg-infosites --vo $VO dliLocal || exit_failure

myecho "VO Boxes"
lcg-infosites --vo $VO vobox || exit_failure

myecho "FTS servers"
lcg-infosites --vo $VO fts || exit_failure

echo ""
echo "    === test PASSED === "
exit 0;

