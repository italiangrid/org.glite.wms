#!/bin/sh
#
# Create and register, get a checksum, and then remove, a GRID file using lcg data management tools.
# Repeat with -l -g -P options
#
# See comments in lcg-tests-functions.sh for explanation of command line options.
#
# Author: Gianni Pucciani

. $(dirname $0)/lcg-tests-functions.sh

echo ""
echo "    === lcg-cr, lcg-lr, lcg-get-checksum, lcg-rep lcg-del test ===    "
echo ""

# ... prepare

lcg_test_startup $@

# ... I. Create grid file and get SURL

run_command lcg-cr $LCG_CR_OPTIONS_BDII $LOCAL_FILE_URI
extract_guid GUID "$OUTPUT"


run_command lcg-lr $VERBOSE $VO_OPTIONS $GUID
extract_surl SURL "$OUTPUT"
convert_to_full_surl SURL

#... II. Get Checksum

run_command lcg-get-checksum $SURL --checksum-type md5
extract_checksum checksum1 "$OUTPUT"


#... III. Replicate file and compare checksum
run_command lcg-rep $LCG_REP_OPTIONS_BDII -d $SE2 $GUID
run_command lcg-lr $VERBOSE $VO_OPTIONS $GUID
SURL_SE2=`grep "/$SE2/" <<<"${OUTPUT}"`
myecho "SURL_SE2 :" $SURL_SE2
convert_to_full_surl SURL_SE2

if [ -z "$SURL_SE2" ]; then
  myecho "Could not extract second SURL from lcg-lr output"
  myexit 1
fi

run_command lcg-get-checksum $SURL_SE2 --checksum-type md5
extract_checksum checksum2 "$OUTPUT"

if [ "x$checksum1" != "x$checksum2" ]; then
  myecho "Checksums are different!"
  myecho $checksum1 $checksum2
  myexit 1
fi

#... IV. Delete all replicas (done by cleanup called by myexit).

myexit 0
