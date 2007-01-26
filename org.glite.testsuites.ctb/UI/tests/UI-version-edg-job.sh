#!/bin/bash

# Try gLite EDG job submission commands with --version

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

echo "    === Version test of edg-job-*  ===     "
echo ""

test_version_or_exit edg-job-submit
test_version_or_exit edg-job-status
test_version_or_exit edg-job-list-match
test_version_or_exit edg-job-cancel
test_version_or_exit edg-job-attach
test_version_or_exit edg-job-get-output
test_version_or_exit edg-job-get-logging-info
test_version_or_exit edg-job-get-chkpt

echo "    ===    seems Ok    ===    "
exit 0
