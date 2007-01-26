#!/bin/bash

# Try MyProxy commands with --version

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

function test_version_or_exit() {

echo "testing version of $1 ..."

if $1 --version; then
   echo ""
else
   echo "Error! Problem getting version of $1!"
   echo ""
#   exit 1
   return 1
fi

}

echo "    === Version test of myproxy-* === "
echo ""

test_version_or_exit myproxy-init
test_version_or_exit myproxy-info
test_version_or_exit myproxy-destroy
test_version_or_exit myproxy-server
test_version_or_exit myproxy-get-delegation
test_version_or_exit myproxy-change-pass-phrase

if [ $? -ne 0 ]; then
  echo "ERROR: Last command returned non-zero exit status ! "
  echo " == test NOT passed == "
  exit 1
else
  echo " == all Ok == "
fi

exit 0

