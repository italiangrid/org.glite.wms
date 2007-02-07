#!/bin/tcsh

# This test is meant to reveal problems with gLite-specific shell variables.
# Failure is returned when a wrongly-pointing variable is found.
# For undefined or empty variables a warning is reported.

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo " === gLite environment test === "

foreach VAR_NAME_TEST ( GLITE_LOCATION GLITE_LOCATION_VAR GLITE_LOCATION_LOG GLITE_LOCATION_TMP GLITE_WMS_LOCATION \
			EDG_LOCATION EDG_LOCATION_VAR EDG_WL_LOCATION EDG_WL_LOCATION_VAR \
			GLOBUS_LOCATION GPT_LOCATION \
			LCG_LOCATION LCG_LOCATION_VAR )

  set DIR_NAME_TEST=`printenv $VAR_NAME_TEST`

  if ( -z "$DIR_NAME_TEST" ) then
    echo "WARNING: $VAR_NAME_TEST is not defined"
  else if ( -d "$DIR_NAME_TEST" ) then
    echo "info   :  $VAR_NAME_TEST  \t= $DIR_NAME_TEST"
  else
      echo "ERROR  : $VAR_NAME_TEST is set to non-existing location $DIR_NAME_TEST"
      echo " === test NOT passed === "
      exit 1
  endif
end


echo " === test PASSED === "
exit 0
