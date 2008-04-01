#!/bin/sh
#
# Test of lcg-cr involving gsiftp TURLs. Also tested are lcg-gt and lcg-sd.
#
# Upload file to the GRID, get TURL, try lcg-cr with gsiftp and with different options
#
# See comments in lcg-tests-functions.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lcg-tests-functions.sh

echo ""
echo "    === test of lcg-cr with gsiftp, lcg-gt and lcg-sd ===    "
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

# ... IV. Get TURL

run_command lcg-gt $LCG_GT_OPTIONS $SURL gsiftp
extract_gsiftp_turl TURL "$OUTPUT"
extract_reqid REQID "$OUTPUT"

# ... V. Make a copy of this file and register it in the catalog

LFN2=${LFN}_another_copy
SURL2=${SURL}_another_copy

myecho "SURL2 : $SURL2"

run_command lcg-cr $LCG_CR_OPTIONS_BASIC -d $SURL2 $TURL
extract_guid GUID2 "$OUTPUT"
run_command lcg-del $LCG_DEL_OPTIONS $SURL2

run_command lcg-cr $LCG_CR_OPTIONS_BASIC -d $SURL2 -l $LFN2 $TURL
run_command lcg-del $LCG_DEL_OPTIONS $SURL2

run_command lcg-cr $LCG_CR_OPTIONS_BASIC -d $SURL2 -g $GUID2 $TURL
run_command lcg-del $LCG_DEL_OPTIONS $SURL2

run_command lcg-cr $LCG_CR_OPTIONS_BASIC -d $SURL2 -l $LFN2 -g $GUID2 $TURL
run_command lcg-del $LCG_DEL_OPTIONS $SURL2

SURL2=""

# ... VI. lcg-cr with destination specified by hostname (requires BDII queries)

run_command lcg-cr $LCG_CR_OPTIONS_BDII $TURL
extract_guid GUID2 "$OUTPUT"
run_command lcg-del $LCG_DEL_OPTIONS_BDII -a $GUID2

run_command lcg-cr $LCG_CR_OPTIONS_BDII -l $LFN2 $TURL
run_command lcg-del $LCG_DEL_OPTIONS_BDII -a $LFN2

run_command lcg-cr $LCG_CR_OPTIONS_BDII -g $GUID2 $TURL
run_command lcg-del $LCG_DEL_OPTIONS_BDII -a $GUID2

run_command lcg-cr $LCG_CR_OPTIONS_BDII -g $GUID2 -l $LFN2 $TURL
run_command lcg-del $LCG_DEL_OPTIONS_BDII -a $GUID2

# ... VII. Set file status to Done

run_command lcg-sd $LCG_SD_OPTIONS $SURL $REQID 0 0

# ... Remove file and exit with success

myexit 0
