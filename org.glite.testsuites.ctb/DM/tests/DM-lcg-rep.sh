#!/bin/sh
#
# Test of lcg data management tools: lcg-rep & lcg-del
# 
# Replication is always from SE1 towards SE2.
# Exchange the hostnames to test the inverse direction.
#
# See comments in lcg-tests-functions.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lcg-tests-functions.sh

echo ""
echo "    === test of lcg-rep & lcg-del ===    "
echo ""

# ... prepare

lcg_test_startup $@

if [ -z "$SE2" ]; then
  echo ""; myecho "Please provide second SE hostname in command line"; echo ""
  usage 1
fi

# ... I. Create and register a GRID file, Get LFN & SURL

run_command lcg-cr $LCG_CR_OPTIONS_BDII $LOCAL_FILE_URI
extract_guid GUID "$OUTPUT"

run_command lcg-la $VERBOSE $VO_OPTIONS $GUID 
extract_lfn LFN "$OUTPUT"

run_command lcg-lr $VERBOSE $VO_OPTIONS $GUID
extract_surl SURL "$OUTPUT"
convert_to_full_surl SURL

# ... II. Replicate file
run_command lcg-rep $LCG_REP_OPTIONS_BDII -d $SE2 $GUID

run_command lcg-lr $VERBOSE $VO_OPTIONS $GUID
SURL_SE2=`grep "/$SE2/" <<<"${OUTPUT}"`
myecho "SURL_SE2 :" $SURL_SE2
convert_to_full_surl SURL_SE2

if [ -z "$SURL_SE2" ]; then
  myecho "Could not extract second SURL from lcg-lr output"
  myexit 1
fi

run_command lcg-del $LCG_DEL_OPTIONS_BDII -s $SE2 $LFN

#run_command lcg-rep $LCG_REP_OPTIONS_BDII -d $SE2 $LFN
#run_command lcg-lr $VERBOSE $VO_OPTIONS $GUID
#run_command lcg-del $LCG_DEL_OPTIONS_BDII -s $SE2 $GUID
sleep 120

run_command lcg-rep $LCG_REP_OPTIONS_BDII -d $SE2 $SURL
run_command lcg-lr $VERBOSE $VO_OPTIONS $GUID
run_command lcg-del $LCG_DEL_OPTIONS_BDII -s $SE2 $GUID

SURL_SE2=${SURL_SE2}_copy1
run_command lcg-rep $LCG_REP_OPTIONS_BDII -d $SURL_SE2 $GUID
run_command lcg-lr $VERBOSE $VO_OPTIONS $GUID
run_command lcg-del $LCG_DEL_OPTIONS $SURL_SE2

SURL_SE2=${SURL_SE2}_copy2
run_command lcg-rep $LCG_REP_OPTIONS_BDII -d $SURL_SE2 $LFN
run_command lcg-lr $VERBOSE $VO_OPTIONS $GUID
run_command lcg-del $LCG_DEL_OPTIONS $SURL_SE2

SURL_SE2=${SURL_SE2}_copy3
run_command lcg-rep $LCG_REP_OPTIONS -d $SURL_SE2 $SURL
run_command lcg-lr $VERBOSE $VO_OPTIONS $GUID
run_command lcg-del $LCG_DEL_OPTIONS $SURL_SE2

# ... Remove file and exit with success

myexit 0
