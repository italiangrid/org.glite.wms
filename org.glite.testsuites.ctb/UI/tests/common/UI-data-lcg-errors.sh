#!/bin/sh
#
# Test that the commands handle errors correctly
# Use a nonexistent SE
# Store nonexistent files
# Store files to which the user doesnt have access
# Use wrong GUIS/SURLS for access
#
# See comments in lcg-tests-common.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lcg-tests-common.sh

echo ""
echo "    === lcg-cr with nonexisting server ===    "
echo ""

# ... define VO_OPTIONS, SE_HOST, LOCAL_FILE_URI

lcg_test_startup $@

# ... I. Create and register a GRID file

echo ""; myecho "lcg-cr $VERBOSE $VO_OPTIONS -d no.such.server.cern.ch $LOCAL_FILE_URI"; echo ""
lcg-cr $VERBOSE $VO_OPTIONS -d no.such.server.cern.ch $LOCAL_FILE_URI 

if [ $? -eq 0 ] ; then
  myecho "ERROR: lcg-cr does not give an error on a nonexisting SE"
  myexit 1
fi

myecho "lcg-cr correctly gives an error on a nonexistent server" 

echo ""
echo "    === lcg-cr with nonexisting file ===    "
echo ""


# ... I. Create and register a GRID file

MY_FILE=`mktemp`
rm $MY_FILE

echo ""; myecho "lcg-cr $VERBOSE $VO_OPTIONS -d $SE_HOST file:$MY_FILE"; echo ""
lcg-cr $VERBOSE $VO_OPTIONS -d $SE_HOST file:$MY_FILE 

if [ $? -eq 0 ] ; then
  myecho "ERROR: lcg-cr does not give an error on nonexisting source file"
  myexit 1
fi

myecho "lcg-cr correctly gives an error on a nonexistent source file" 

echo ""
echo "    === lcg-cr with file withour read permissions ===    "
echo ""


# ... I. Create and register a GRID file

MY_FILE=`mktemp`
echo "TEsttsts" >> $MY_FILE
chmod 200 $MY_FILE

echo ""; myecho "lcg-cr $VERBOSE $VO_OPTIONS -d $SE_HOST file:$MY_FILE"; echo ""
lcg-cr $VERBOSE $VO_OPTIONS -d $SE_HOST file:$MY_FILE 

if [ $? -eq 0 ] ; then
  myecho "ERROR: lcg-cr does not give an error on read-protected source file"
  myexit 1
fi

myecho "lcg-cr correctly gives an error on a read-protected source file" 

rm $MY_FILE

echo ""
echo "    === lcg-cp with bad SURSL ===    "
echo ""


# ... I. Create and register a GRID file

LOCAL_FILE_BACK=${LOCAL_FILE}_from_se


echo ""; myecho "lcg-cp $VERBOSE $VO_OPTIONS guid:12345678-90ab-cdef-1234-56789abcdef2 file:$LOCAL_FILE_BACK"; echo ""
lcg-cp $VERBOSE $VO_OPTIONS guid:12345678-90ab-cdef-1234-56789abcdef2 file:$LOCAL_FILE_BACK

if [ $? -eq 0 ]; then
  myecho "ERROR: lcg-cp does not give errors on a bad GUID"
  myexit 1
fi

myecho "lcg-cp correctly gives errors on a bad GUID"

myexit 0
