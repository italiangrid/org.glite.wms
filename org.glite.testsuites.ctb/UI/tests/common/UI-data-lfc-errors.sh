#!/bin/sh
#
# Create a direcotry in the root of LFC where you shouldn't have permission, and 
# use a nonexistent lfc server to see if commands behave properly
#
# See comments in lfc-tests-common.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>ff
# Version: $Id$

. $(dirname $0)/lfc-tests-common.sh

echo ""
echo "    === lfc-mkdir test for a bad path===    "
echo ""

# ... define LFC_HOST TEST_DIR

lfc_test_startup $@

# ... I. Create directory

echo ""; myecho "lfc-mkdir /this/doesnt/exist"; echo ""
lfc-mkdir /this/doesnt/exist

if [ $? -eq 0 ]; then
  myecho "ERROR: The command returns success for creating an impossible directory"
  exit_failure
fi

myecho "lfc-mkdir correctly reports an error for trying to create an impossible directory" 

myecho "lfc-ls with bad server" 

LFC_HOST="no.such.server.cern.ch" lfc-ls /

if [ $? -eq 0 ]; then
  myecho "ERROR: lfc-ls does not report error for using a nonexisting server"
  exit_failure
fi

myecho "lfc-ls correctly reports error when a nonexisting server is used" 

exit_success
