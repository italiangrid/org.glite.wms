#!/bin/sh
#
# Test of lcg-ls, lcg-getturls
# (this test needs gLite >= 3.1)
#
# See comments in lcg-tests-functions.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lcg-tests-functions.sh

echo ""
echo "    === lcg-ls, lcg-getturls test ===    "
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

# ... II. List file with lcg-ls

run_command lcg-ls $LCG_LS_OPTIONS $SURL
run_command lcg-ls -l $LCG_LS_OPTIONS $SURL

# ... III. List parent directory

DIR_SURL=$(dirname $SURL)

run_command lcg-ls $LCG_LS_OPTIONS $DIR_SURL
run_command lcg-ls -l $LCG_LS_OPTIONS $DIR_SURL
run_command lcg-ls -d $LCG_LS_OPTIONS $DIR_SURL
run_command lcg-ls -d -l $LCG_LS_OPTIONS $DIR_SURL

# ... IV. Get TURLs (all available)

run_command lcg-getturls $LCG_GT_OPTIONS $SURL

# ... Remove the GRID file referred by GUID and exit

myexit 0
