#!/bin/sh
#
# A test of lcg data management tools related to aliases: lcg-la, lcg-aa, lcg-ra
#
# Upload a file to the GRID, list alias, create new alias, list again, remove alias;
# create another alias, remove the file using this alias with lcg-del
#
# See comments in lcg-tests-functions.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lcg-tests-functions.sh

echo ""
echo "    === lcg-la, -aa, -ra test ===    "
echo ""

# ... prepare

lcg_test_startup $@

# ... I. Create grid file

run_command lcg-cr $LCG_CR_OPTIONS_BDII $LOCAL_FILE_URI
extract_guid GUID "$OUTPUT"

# ... II. List alias (only primary LFN at this point)

run_command lcg-la $VERBOSE $VO_OPTIONS $GUID 
extract_lfn LFN "$OUTPUT"

# ... III. Create an alias

ALIAS=${LFN}_alias
run_command lcg-aa $VERBOSE $VO_OPTIONS $GUID $ALIAS

# ... IV. Find replica by alias (could be done by GUID or primary LFN of course, but this is a different story)

run_command lcg-lr $VERBOSE $VO_OPTIONS $ALIAS
extract_surl SURL "$OUTPUT"

# ... V. List aliases (primary LFN + the alias) by GUID, primary LFN, alias and SURL (should produce the same output though)

run_command lcg-la $VERBOSE $VO_OPTIONS $GUID
run_command lcg-la $VERBOSE $VO_OPTIONS $LFN
run_command lcg-la $VERBOSE $VO_OPTIONS $ALIAS
run_command lcg-la $VERBOSE $VO_OPTIONS $SURL

# ... VI. Remove alias

run_command lcg-ra $VERBOSE $VO_OPTIONS $GUID $ALIAS

# ... VII. List alias (=primary LFN)

run_command lcg-la $VERBOSE $VO_OPTIONS $GUID

# ... VIII. Create another alias

ALIAS2=${LFN}_alias2
run_command lcg-aa $VERBOSE $VO_OPTIONS $GUID $ALIAS2

# ... IX. Remove file by alias

run_command lcg-del $LCG_DEL_OPTIONS_BDII -s $SE_HOST $ALIAS2
GUID=""

# ... Exit with success

myexit 0
