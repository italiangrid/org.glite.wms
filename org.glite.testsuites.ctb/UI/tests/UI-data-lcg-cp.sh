#!/bin/sh
#
# Upload, download and remove a GRID file using lcg data management tools.
#
# See comments in lcg-tests-common.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lcg-tests-common.sh

echo ""
echo "    === lcg-cr, -cp, -del test ===    "
echo ""

# ... define VO_OPTIONS, SE_HOST, LOCAL_FILE_URI

lcg_test_startup $@

# ... I. Create and register a GRID file

echo ""; myecho "lcg-cr $VO_OPTIONS -d $SE_HOST $LOCAL_FILE_URI"; echo ""
GUID=`lcg-cr $VO_OPTIONS -d $SE_HOST $LOCAL_FILE_URI`

if [ $? -eq 0 ]; then
  echo "$GUID"
  echo ""
  myecho "GRID file $GUID has been successfully created"
else
  echo "$GUID"
  myecho "lcg-cr failed"
  myexit 1
fi

# ... II. Download the GRID file

LOCAL_FILE_BACK=${LOCAL_FILE}_from_se

echo ""; myecho "lcg-cp -v $VO_OPTIONS $GUID file:$LOCAL_FILE_BACK"; echo ""
lcg-cp -v $VO_OPTIONS $GUID file:$LOCAL_FILE_BACK

if [ $? -ne 0 ]; then
  myecho "lcg-cp failed"
  myexit 1
fi

echo ""
myecho " ---    $LOCAL_FILE_BACK     --- "
cat $LOCAL_FILE_BACK
myecho " --- end of $LOCAL_FILE_BACK --- "
rm -f $LOCAL_FILE_BACK

# ... III. Remove the GRID file referred by GUID and exit

myexit 0
