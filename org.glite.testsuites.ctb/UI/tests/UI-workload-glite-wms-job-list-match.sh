#!/bin/bash

# A job-list-match test for the gLite-WMS submissoin system.

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

echo ""
echo "    === test of glite-wms-job-list-match ===    "
echo ""

JDLFILE=$(dirname $0)/hostname.jdl

echo "--- will use JDL file as follows --- "
echo ""
cat $JDLFILE
echo "--- end of JDL file --- "
echo ""

echo " # glite-wms-job-list-match test # trying glite-wms-job-list-match"
echo ""

glite-wms-job-list-match -a --rank $JDLFILE

if [ $? -ne 0 ]; then

  echo " # glite-wms-job-list-match test # ERROR: job-list-match failed"

  echo ""
  echo " *** something went wrong *** "
  echo " *** test NOT passed *** "
  exit 1

fi

echo "    === test PASSED === "
exit 0
