#!/bin/bash

# Try gLite job submission commands with --version

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

function test_version_or_exit() {

echo "testing version of $1 ..."

if $1 --version; then
   echo ""
else
   echo "Error! Problem getting version of $1!"
   exit 1
fi

}

echo "    === Version test of glite-job-*  ===     "
echo ""

test_version_or_exit glite-job-submit
test_version_or_exit glite-job-status
test_version_or_exit glite-job-list-match
test_version_or_exit glite-job-cancel
test_version_or_exit glite-job-attach
test_version_or_exit glite-job-output
test_version_or_exit glite-job-logging-info
test_version_or_exit glite-job-get-chkpt

echo "    ===    seems Ok    ===    "
exit 0
