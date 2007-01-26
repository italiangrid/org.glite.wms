#!/bin/bash

# Try gLite WMS job submission commands with --version

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

function test_version_or_exit() {

echo "    --- testing version of $1 ---   "

if $1 --version; then
   echo ""
else
   echo "Error! Problem getting version of $1!"
   echo "Please make sure that you have a valid proxy with VO extension"
   echo "and the certificate of the used VOMS server is installed in your system"
   echo " === test NOT passed === "
   exit 1
fi

}

echo "    === Version test of glite-wms-job-*  ===     "
echo ""

echo "this test requires valid proxy, so checking proxy first"

voms-proxy-info -all

if [ $? -ne 0 ]; then
   echo "Error! No proxy valid for this operation found."
   echo "Please create a proxy with voms-proxy-init -voms <yourvo> and retry the test."
   exit 1
fi

echo ""

# ... these are "real" glite-wms-job-* commands
# ... require a valid proxy to get server version
test_version_or_exit glite-wms-job-submit
test_version_or_exit glite-wms-job-list-match
test_version_or_exit glite-wms-job-cancel
test_version_or_exit glite-wms-job-output
test_version_or_exit glite-wms-job-delegate-proxy
test_version_or_exit glite-wms-job-perusal

# ... these are normally symbolic links to the corresponding glite-job-* commands
test_version_or_exit glite-wms-job-status
test_version_or_exit glite-wms-job-attach
test_version_or_exit glite-wms-job-logging-info
test_version_or_exit glite-wms-job-get-chkpt

echo "    ===    seems Ok    ===    "
exit 0
