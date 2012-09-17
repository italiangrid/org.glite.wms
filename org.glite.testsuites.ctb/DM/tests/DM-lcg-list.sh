#!/bin/sh
#
# Upload a file to the GRID, list replica, list GUID for the replica,
# and delete the file using lcg data management tools.
#
# See comments in lcg-tests-functions.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lcg-tests-functions.sh

echo ""
echo "    === lcg-cr, -lr, -lg, -la test ===    "
echo ""

# ... prepare

lcg_test_startup $@

# ... I. Create grid file

run_command lcg-cr $LCG_CR_OPTIONS_BDII $LOCAL_FILE_URI
extract_guid GUID "$OUTPUT"

# ... II. Get LFN by GUID

run_command lcg-la $VERBOSE $VO_OPTIONS $GUID 
extract_lfn LFN "$OUTPUT"

# ... III. Get replica (SURL) by GUID, LFN and SURL, compare with each other

run_command lcg-lr $VERBOSE $VO_OPTIONS $GUID
SURL=$OUTPUT
echo "DEBUG surl=$SURL"

run_command lcg-lr $VERBOSE $VO_OPTIONS $LFN

if [ "$OUTPUT" != "$SURL" ]; then
  echo "The SURLs obtained by the two methods seem to differ !"
  myexit 1
fi

run_command lcg-lr $VERBOSE $VO_OPTIONS $SURL

if [ "$OUTPUT" != "$SURL" ]; then
  echo "The SURLs obtained by the two methods seem to differ !"
  myexit 1
fi

# ... IV. Get GUID by LFN and SURL, compare with original GUID

run_command lcg-lg $VERBOSE $VO_OPTIONS $LFN
extract_guid GUID2 "$OUTPUT"

if [ "$GUID" != "$GUID2" ]; then
  echo "wrong GUID \"$GUID2\" != \"$GUID\""
  myexit 1
fi

run_command lcg-lg $VERBOSE $VO_OPTIONS $SURL
extract_guid GUID3 "$OUTPUT"

if [ "$GUID" != "$GUID3" ]; then
  echo "wrong GUID \"$GUID3\" != \"$GUID\""
  myexit 1
fi

# ... V. Get LFN by LFN (!) and SURL, compare with LFN obtained by GUID

run_command lcg-la $VERBOSE $VO_OPTIONS $LFN
extract_lfn LFN2 "$OUTPUT"

if [ "$LFN2" != "$LFN" ]; then
  echo "wrong LFN \"$LFN2\" != \"$LFN\""
  myexit 1
fi

run_command lcg-la $VERBOSE $VO_OPTIONS $SURL
extract_lfn LFN3 "$OUTPUT"

if [ "$LFN3" != "$LFN" ]; then
  echo "wrong LFN \"$LFN3\" != \"$LFN\""
  myexit 1
fi

# ... Remove the GRID file referred by GUID and exit

myexit 0
