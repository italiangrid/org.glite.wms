#!/bin/sh

# This test is meant to reveal problems with gLite-specific shell variables.
# Failure is returned when a wrongly-pointing variable is found.
# For undefined or empty variables a warning is reported.

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

echo " === gLite environment test === "

function testpath() {
  if [ -n "$2" ]; then
    if [ -d "$2" ]; then
    echo -e "info   :  $1  \t= $2"
    else
      echo "ERROR  :  $1 is set to non-existing location $2"
      echo " === test NOT passed === "
      exit 1
    fi
  else
    echo "WARNING: $1 is not defined"
  fi
}

testpath GLITE_LOCATION      $GLITE_LOCATION
testpath GLITE_LOCATION_VAR  $GLITE_LOCATION_VAR
testpath GLITE_LOCATION_LOG  $GLITE_LOCATION_LOG
testpath GLITE_LOCATION_TMP  $GLITE_LOCATION_TMP

testpath GLITE_WMS_LOCATION  $GLITE_WMS_LOCATION

testpath EDG_LOCATION	     $EDG_LOCATION
testpath EDG_LOCATION_VAR    $EDG_LOCATION_VAR

testpath EDG_WL_LOCATION     $EDG_WL_LOCATION
testpath EDG_WL_LOCATION_VAR $EDG_WL_LOCATION_VAR

testpath GLOBUS_LOCATION     $GLOBUS_LOCATION
testpath GPT_LOCATION	     $GPT_LOCATION

testpath LCG_LOCATION	     $LCG_LOCATION
testpath LCG_LOCATION_VAR    $LCG_LOCATION_VAR

echo " === test PASSED === "
exit 0
