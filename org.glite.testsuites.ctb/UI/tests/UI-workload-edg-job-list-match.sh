#!/bin/sh

# A job-list-match test for the EDG submission system.

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

echo ""
echo "    === test of edg-job-list-match ===    "
echo ""

JDLFILE=$(dirname $0)/hostname.jdl

echo "--- will use JDL file as follows --- "
echo ""
cat $JDLFILE
echo "--- end of JDL file --- "
echo ""

echo " # edg-job-list-match test # trying edg-job-list-match"
echo ""

edg-job-list-match --rank $JDLFILE

if [ $? -ne 0 ]; then

  echo " # edg-job-list-match test # ERROR: job-list-match failed"

  echo ""
  echo " *** something went wrong *** "
  echo " *** test NOT passed *** "
  exit 1

fi

echo "    === test PASSED === "
exit 0
