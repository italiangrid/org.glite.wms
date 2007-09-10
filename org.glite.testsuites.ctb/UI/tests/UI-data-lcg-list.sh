#!/bin/sh
#
# Upload a file to the GRID, list replica, list GUID for the replica, get TURL,
# and delete the file using lcg data management tools.
#
# See comments in lcg-tests-common.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lcg-tests-common.sh

echo ""
echo "    === lcg-cr, -lr, -lg, -gt test ===    "
echo ""

# ... define VO_OPTIONS, SE_HOST, LOCAL_FILE_URI

lcg_test_startup $@

# ... I. Create and register a GRID file

echo ""; myecho "lcg-cr $VERBOSE $VO_OPTIONS -d $SE_HOST $LOCAL_FILE_URI"; echo ""
lcg-cr $VERBOSE $VO_OPTIONS -d $SE_HOST $LOCAL_FILE_URI > $MYTMPDIR/guid.$$
GUID=""
GUID=`cat $MYTMPDIR/guid.$$ | grep -E "^ *guid:|^ *lfn:|^ *srm:|^ *sfn:"` > $MYTMPDIR/guid.$$
cat $MYTMPDIR/guid.$$
rm -rf $MYTMPDIR/guid.$$

if [ $GUID ]; then
  echo ""
  myecho "GRID file $GUID has been successfully created"
else
  echo "$GUID"
  myecho "lcg-cr failed"
  myexit 1
fi

# ... II. List replicas

echo ""; myecho "lcg-lr $VERBOSE $VO_OPTIONS $GUID"; echo ""
SURL=`lcg-lr $VERBOSE $VO_OPTIONS $GUID`

if [ $? -eq 0 ]; then
  echo "$SURL"
else
  echo "$SURL"
  myecho "lcg-lr failed"
  myexit 1
fi

# ... III. List GUIDs

echo ""; myecho "lcg-lg $VERBOSE $VO_OPTIONS $SURL"; echo ""
GUID2=`lcg-lg $VERBOSE $VO_OPTIONS $SURL`

if [ $? -eq 0 ]; then
  echo "$GUID2"
else
  echo "$GUID2"
  myecho "lcg-lg failed"
  myexit 1
fi

if [ "$GUID" != "$GUID2" ]; then
  echo "wrong GUID \"$GUID2\" != \"$GUID\""
  myexit 1
fi

# ... IV. Get TURL

echo ""; myecho "lcg-gt $VERBOSE $SURL gsiftp"; echo ""
lcg-gt $VERBOSE $SURL gsiftp

if [ $? -ne 0 ]; then
  myecho "Could not get TURL for $SURL"
  myexit 1
fi

# ... V. Remove the GRID file referred by GUID and exit

myexit 0
