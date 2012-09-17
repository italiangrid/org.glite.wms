#!/bin/sh
#
# Test of lcg-cp
#
# Upload, download and remove a GRID file using lcg data management tools.
#
# NB: multistream lcg-cp requires certain port range open in the firewall on the destination side.
# This applies also to the file: protocol
#
# See comments in lcg-tests-functions.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lcg-tests-functions.sh

function dump_file() {

  THE_FILE=$1
  if [ -f $THE_FILE ]; then
    myecho " ---    $THE_FILE     --- "
    cat $THE_FILE || myexit 1
    myecho " --- end of $THE_FILE --- "
    rm -f $THE_FILE
  else
    myexit 1
  fi
}

echo ""
echo "    === lcg-cr, -cp, -del test ===    "
echo ""

# ... prepare

lcg_test_startup $@

# ... I. Create and register a GRID file

run_command lcg-cr $LCG_CR_OPTIONS_BDII $LOCAL_FILE_URI
extract_guid GUID "$OUTPUT"

run_command lcg-la $VERBOSE $VO_OPTIONS $GUID 
extract_lfn LFN "$OUTPUT"

run_command lcg-lr $VERBOSE $VO_OPTIONS $GUID
extract_surl SURL "$OUTPUT"
convert_to_full_surl SURL

# ... II. Download the GRID file with lcg-cp

LOCAL_FILE_BACK=${LOCAL_FILE}_from_se
run_command lcg-cp $LCG_CP_OPTIONS $SURL file:$LOCAL_FILE_BACK   # does not need BDII
dump_file $LOCAL_FILE_BACK

LOCAL_FILE_BACK_2=${LOCAL_FILE}_from_se_2
run_command lcg-cp $LCG_CP_OPTIONS_BDII $GUID file:$LOCAL_FILE_BACK_2 # needs BDII
dump_file $LOCAL_FILE_BACK_2

LOCAL_FILE_BACK_3=${LOCAL_FILE}_from_se_3
run_command lcg-cp $LCG_CP_OPTIONS_BDII $LFN file:$LOCAL_FILE_BACK_3  # needs BDII
dump_file $LOCAL_FILE_BACK_3

# ... Remove file referred by GUID and exit with success

myexit 0
