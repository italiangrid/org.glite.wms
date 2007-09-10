#!/bin/sh
#
# Create and register, and then remove, a GRID file using lcg data management tools.
#
# See comments in lcg-tests-common.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lcg-tests-common.sh

echo ""
echo "    === lcg-cr, -del test ===    "
echo ""

# ... define VO_OPTIONS, SE_HOST, LOCAL_FILE_URI, VERBOSE

lcg_test_startup $@

# ... I. Create and register a GRID file

echo ""; myecho "lcg-cr $VERBOSE $VO_OPTIONS -d $SE_HOST $LOCAL_FILE_URI"; echo ""
lcg-cr $VERBOSE $VO_OPTIONS -d $SE_HOST $LOCAL_FILE_URI > $MYTMPDIR/guid.$$
GUID=""
GUID=`cat $MYTMPDIR/guid.$$ | grep -E "^ *guid:|^ *lfn:|^ *srm:|^ *sfn:"` > $MYTMPDIR/guid.$$
cat $MYTMPDIR/guid.$$
rm -f $MYTMPDIR/guid.$$

if [ $GUID ]; then
  echo ""
  myecho "GRID file has been successfully created with GUID $GUID"
else
  echo "$GUID"
  myecho "lcg-cr failed"
  myexit 1
fi

# ... II. Remove the GRID file referred by GUID and exit with success

myexit 0
