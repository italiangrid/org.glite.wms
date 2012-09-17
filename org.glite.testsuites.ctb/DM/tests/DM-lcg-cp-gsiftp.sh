#!/bin/sh
#
# Test of lcg-cp involving gsiftp
#
# Upload file to the GRID, get TURL, copy back and forth, remove
#
# See comments in lcg-tests-functions.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lcg-tests-functions.sh

echo ""
echo "    === test of lcg-cp with gsiftp, lcg-gt and lcg-rf ===    "
echo ""

# ... prepare

lcg_test_startup $@

# ... I. Create grid file

run_command lcg-cr $LCG_CR_OPTIONS_BDII $LOCAL_FILE_URI
extract_guid GUID "$OUTPUT"

run_command lcg-la $VERBOSE $VO_OPTIONS $GUID 
extract_lfn LFN "$OUTPUT"

run_command lcg-lr $VERBOSE $VO_OPTIONS $GUID
extract_surl SURL "$OUTPUT"
convert_to_full_surl SURL

# ... II. Get TURL

run_command lcg-getturls $LCG_GT_OPTIONS  -p gsiftp $SURL
extract_gsiftp_turl TURL "$OUTPUT"

# ... III. Make a copy of this file and register it in the catalog

SURL2=${SURL}_lcg_cp_test
run_command lcg-cp $LCG_CP_OPTIONS $TURL $SURL2
run_command lcg-rf $LCG_RF_OPTIONS $SURL2

# ... VI. Get TURL for the second file

run_command lcg-gt $LCG_GT_OPTIONS $SURL2 gsiftp
extract_gsiftp_turl TURL2 "$OUTPUT"

# ... V. Try owerwrite the new file if requested (not every SE allows this!)

#export TEST_OVERWRITE="yes"

if [ "$TEST_OVERWRITE" == "yes" ]; then

  myecho "Now trying to overwrite file via gsiftp"
  myecho "Beware: this test will fail if SE does not allow to overwrite files - and this may be normal!"
  run_command lcg-cp $LCG_CP_OPTIONS $LOCAL_FILE_URI $TURL2
  run_command lcg-cp $LCG_CP_OPTIONS $TURL $TURL2
  run_command lcg-cp $LCG_CP_OPTIONS $SURL $TURL2
  run_command lcg-cp $LCG_CP_OPTIONS_BDII $GUID $TURL2
  run_command lcg-cp $LCG_CP_OPTIONS_BDII $LFN $TURL2

fi

# ... VI. Get file back

LOCAL_FILE_BACK=${LOCAL_FILE}_is_back
run_command lcg-cp $LCG_CP_OPTIONS $TURL file:$LOCAL_FILE_BACK

myecho "the file obtained from se:"
cat $LOCAL_FILE_BACK
rm -f $LOCAL_FILE_BACK

# ... VII. Set file status to Done

#run_command lcg-sd $LCG_SD_OPTIONS $SURL

# ... Remove the files and exit with success

myexit 0
