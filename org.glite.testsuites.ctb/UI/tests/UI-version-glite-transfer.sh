#!/bin/bash

# Try gLite FTS commands with -V

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

function test_version_or_exit() {

echo "testing version of $1 ..."

if $1 -V; then
   echo ""
else
   echo "Error! Problem getting version of $1!"
   exit 1
fi

}

echo "    === Version test of glite-transfer-*  ===     "
echo ""

test_version_or_exit glite-transfer-submit
test_version_or_exit glite-transfer-status
test_version_or_exit glite-transfer-list
test_version_or_exit glite-transfer-cancel

test_version_or_exit glite-transfer-channel-add   
test_version_or_exit glite-transfer-channel-list  
test_version_or_exit glite-transfer-channel-set   
test_version_or_exit glite-transfer-channel-signal

echo "    ===    seems Ok    ===    "
exit 0
