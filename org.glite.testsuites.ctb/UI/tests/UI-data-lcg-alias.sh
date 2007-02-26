#!/bin/sh
#
# A test of lcg data management tools:
# Upload a file to the GRID, list alias, create new alias, list again and remove.
#
# See comments in lcg-tests-common.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lcg-tests-common.sh

echo ""
echo "    === lcg-la, -aa, -ra test ===    "
echo ""

# ... define VO_OPTIONS, SE_HOST, LOCAL_FILE_URI

lcg_test_startup $@

# ... I. Create and register a GRID file

echo ""; myecho "lcg-cr -v $VO_OPTIONS -d $SE_HOST $LOCAL_FILE_URI"; echo ""

OUTPUT=`lcg-cr -v $VO_OPTIONS -d $SE_HOST $LOCAL_FILE_URI 2>&1`

if [ $? -ne 0 ]; then
  echo "$OUTPUT"
  myecho "lcg-cr failed"
  myexit 1
fi

echo "$OUTPUT"
echo ""

myecho "GRID file has been created"

GUID=`awk -F ' ' '/guid:/ {print $NF}' <<<"$OUTPUT"`
LFN=`awk -F ' ' '/lfn:/ {print $NF}' <<<"$OUTPUT"`

myecho "GUID: $GUID"
myecho "LFN : $LFN"

# ... II. List alias

echo ""; myecho "lcg-la $VO_OPTIONS $GUID"; echo ""
lcg-la $VO_OPTIONS $GUID || myexit 1

# ... III. Create an alias

ALIAS=${LFN}_alias
echo ""; myecho "lcg-aa $VO_OPTIONS $GUID $ALIAS"; echo ""
lcg-aa $VO_OPTIONS $GUID $ALIAS || myexit 1

# ... IV. List alias

echo ""; myecho "lcg-la $VO_OPTIONS $GUID"; echo ""
lcg-la $VO_OPTIONS $GUID || myexit 1

# ... V. Remove alias

echo ""; myecho "lcg-ra $VO_OPTIONS $ALIAS"; echo ""
lcg-ra $VO_OPTIONS $GUID $ALIAS || myexit 1

# ... VI. List alias

echo ""; myecho "lcg-la $VO_OPTIONS $GUID"; echo ""
lcg-la $VO_OPTIONS $GUID || myexit 1

# ... VII. Remove the GRID file referred by GUID and exit with success

myexit 0
