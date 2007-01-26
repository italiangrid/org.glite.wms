#!/bin/bash

# Try grid-proxy-init/info/destroy and grid-cert-info with -version

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

function test_version_or_exit() {

echo "testing version of $1 ..."

if $1 -version; then
   echo ""
else
   echo "Error! Problem getting version of $1!"
   exit 1
fi

}

echo "    == Version test of grid-proxy-* and grid-cert-info === "
echo ""

test_version_or_exit grid-proxy-init
test_version_or_exit grid-proxy-info
test_version_or_exit grid-proxy-destroy
test_version_or_exit grid-cert-info

echo " == all Ok == "
exit 0

