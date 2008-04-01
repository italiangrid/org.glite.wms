#!/bin/sh
#
# Test of lcg data management tools: lcg-rf, lcg-uf, lcg-cp
# Upload file to the GRID, unregister it, register again with the same GUID (LFN, GUID&LFN), remove;
# upload file without regestering it, then register and remove.
#
# See comments in lcg-tests-functions.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lcg-tests-functions.sh

echo ""
echo "    === lcg-rf, -uf, -cp test ===    "
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

# ... II. Unregister file

run_command lcg-uf $LCG_UF_OPTIONS $GUID $SURL

# ... III. Register file again with the same GUID; with the same LFN; with the same GUID and LFN

run_command lcg-rf $LCG_RF_OPTIONS -g $GUID $SURL
run_command lcg-uf $LCG_UF_OPTIONS $GUID $SURL

run_command lcg-rf $LCG_RF_OPTIONS -g $GUID -l $LFN $SURL
run_command lcg-uf $LCG_UF_OPTIONS $GUID $SURL

run_command lcg-rf $LCG_RF_OPTIONS -l $LFN $SURL

# ... IV. Remove it

run_command lcg-del $LCG_DEL_OPTIONS $SURL
GUID=""

# ... V. Upload file using lcg-cp, without registering it in the catalog

SURL=${SURL}_manual_register

run_command lcg-cp $LCG_CP_OPTIONS $LOCAL_FILE_URI $SURL

# ... VI. Register the file in the catalog

run_command lcg-rf $LCG_RF_OPTIONS $SURL
extract_guid GUID "$OUTPUT"

# ... VII. Finally unregister and remove the file

run_command lcg-uf $LCG_UF_OPTIONS $GUID $SURL
run_command lcg-del $LCG_DEL_OPTIONS --nolfc $SURL
GUID=""

# ... Exit with success

myexit 0
