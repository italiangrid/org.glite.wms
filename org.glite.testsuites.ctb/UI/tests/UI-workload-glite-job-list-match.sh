#!/bin/bash

# A job-list-match test for the gLite submissoin system.

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

echo ""
echo "    === test of glite-job-list-match ===    "
echo ""

JDLFILE=$(dirname $0)/hostname.jdl

echo "--- will use JDL file as follows --- "
echo ""
cat $JDLFILE
echo "--- end of JDL file --- "
echo ""

echo " # glite-job-list-match test # trying glite-job-list-match"
echo ""

glite-job-list-match --rank $JDLFILE

if [ $? -ne 0 ]; then

  echo " # glite-job-list-match test # ERROR: job-list-match failed"

  echo ""
  echo " *** something went wrong *** "
  echo " *** test NOT passed *** "
  exit 1

fi

echo "    === test PASSED === "
exit 0
