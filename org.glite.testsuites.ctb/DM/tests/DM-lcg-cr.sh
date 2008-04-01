#!/bin/sh
#
# Create and register, and then remove, a GRID file using lcg data management tools.
# Repeat with -l -g -P options
#
# See comments in lcg-tests-functions.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lcg-tests-functions.sh

echo ""
echo "    === lcg-cr, -del test ===    "
echo ""

# ... prepare

lcg_test_startup $@

# ... I. Create grid file, then remove it

run_command lcg-cr $LCG_CR_OPTIONS_BDII $LOCAL_FILE_URI
extract_guid GUID "$OUTPUT"

run_command lcg-la $VERBOSE $VO_OPTIONS $GUID 
extract_lfn LFN "$OUTPUT"

run_command lcg-lr $VERBOSE $VO_OPTIONS $GUID
extract_surl SURL "$OUTPUT"
convert_to_full_surl SURL

run_command lcg-del $LCG_DEL_OPTIONS $SURL

# ... II. Try -l, -g options

LFN=${LFN}_cr_test
SURL=${SURL}_cr_test

run_command lcg-cr $LCG_CR_OPTIONS_BDII -l $LFN $LOCAL_FILE_URI
run_command lcg-del $LCG_DEL_OPTIONS_BDII -a $LFN

run_command lcg-cr $LCG_CR_OPTIONS_BDII -g $GUID $LOCAL_FILE_URI
run_command lcg-del $LCG_DEL_OPTIONS_BDII -a $GUID

run_command lcg-cr $LCG_CR_OPTIONS_BDII -g $GUID -l $LFN $LOCAL_FILE_URI
run_command lcg-del $LCG_DEL_OPTIONS_BDII -a $GUID

# ... III. Try lcg-cr with fully qualified SURL

run_command lcg-cr $LCG_CR_OPTIONS_BASIC -d $SURL $LOCAL_FILE_URI
#extract_guid GUID "$OUTPUT"
run_command lcg-del $LCG_DEL_OPTIONS $SURL

run_command lcg-cr $LCG_CR_OPTIONS_BASIC -d $SURL -l $LFN $LOCAL_FILE_URI
run_command lcg-del $LCG_DEL_OPTIONS $SURL

run_command lcg-cr $LCG_CR_OPTIONS_BASIC -d $SURL -g $GUID $LOCAL_FILE_URI
run_command lcg-del $LCG_DEL_OPTIONS $SURL

run_command lcg-cr $LCG_CR_OPTIONS_BASIC -d $SURL -l $LFN -g $GUID $LOCAL_FILE_URI
run_command lcg-del $LCG_DEL_OPTIONS $SURL

# ... IV. Try lcg-cr with relative path

run_command lcg-cr $LCG_CR_OPTIONS_BDII -P $LOCAL_FILE_NAME $LOCAL_FILE_URI
extract_guid GUID "$OUTPUT"

run_command lcg-lr $GUID
run_command lcg-la $GUID

# ... the following works, but SURL given with -d option is then used (-P is ignored) - so no reason to test in combination
# run_command lcg-cr $LCG_CR_OPTIONS_BASIC -d $SURL -P $LOCAL_FILE_NAME $LOCAL_FILE_URI

# ... Remove the GRID file referred by GUID and exit with success

myexit 0
