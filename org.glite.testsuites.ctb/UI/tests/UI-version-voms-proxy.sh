#!/bin/bash

# Try VOMS proxy commands with --version

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

echo "    === Version test of voms-proxy-* ==="
echo ""

test_version_or_exit voms-proxy-init
test_version_or_exit voms-proxy-destroy
test_version_or_exit voms-proxy-info
test_version_or_exit voms-proxy-list
test_version_or_exit voms-proxy-fake

echo " == all Ok == "
exit 0

