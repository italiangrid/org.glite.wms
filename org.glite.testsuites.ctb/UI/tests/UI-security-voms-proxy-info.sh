#!/bin/bash

# Test voms-proxy-info with different options

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

function myexit() {

  if [ $1 -ne 0 ]; then
    echo " *** something went wrong *** "
    echo " *** test NOT passed *** "
    exit $1
  else
    echo ""
    echo "   === test PASSED === "
  fi
   
  exit 0
}

function myecho()
{
  echo "#voms-proxy-info test# $1"
}

echo "    === Option test of voms-proxy-info === "
echo ""
myecho "voms-proxy-info without options ..."
voms-proxy-info || myexit 1

echo ""
myecho "voms-proxy-info -all ..."
voms-proxy-info -all || myexit 1

echo ""
myecho "voms-proxy-info -debug ..."
voms-proxy-info -debug || myexit 1

echo ""
myecho "voms-proxy-info -text ..."
voms-proxy-info -text || myexit 1

echo ""
myecho "voms-proxy-info -subject ..."
voms-proxy-info -subject || myexit 1

echo ""
myecho "voms-proxy-info -issuer ..."
grid-cert-info -issuer || myexit 1

echo ""
myecho "voms-proxy-info -identity ..."
voms-proxy-info -identity || myexit 1

echo ""
myecho "voms-proxy-info -type ..."
voms-proxy-info -type || myexit 1

echo ""
myecho "voms-proxy-info -timeleft ..."
voms-proxy-info -timeleft || myexit 1

echo ""
myecho "voms-proxy-info -strength ..."
voms-proxy-info -strength || myexit 1

echo ""
myecho "voms-proxy-info -path ..."
voms-proxy-info -path || myexit 1

echo ""
myecho "voms-proxy-info -exists -bits 256 ..."
voms-proxy-info -exists -bits 256 && echo "YES" || echo "NO"

echo ""
myecho "voms-proxy-info -exists -bits 512 ..."
voms-proxy-info -exists -bits 512 && echo "YES" || echo "NO"

echo ""
myecho "voms-proxy-info -exists -bits 1024 ..."
voms-proxy-info -exists -bits 1024 && echo "YES" || echo "NO"

echo ""
myecho "voms-proxy-info -exists -valid 1:00 ..."
voms-proxy-info -exists  -valid 1:00 && echo "YES" || echo "NO"

echo ""
myecho "voms-proxy-info -exists -valid 3:00 ..."
voms-proxy-info -exists  -valid 3:00 && echo "YES" || echo "NO"

echo ""
myecho "voms-proxy-info -exists -valid 10:00 ..."
voms-proxy-info -exists  -valid 10:00 && echo "YES" || echo "NO"

echo ""
myecho "voms-proxy-info -exists -valid 24:00 ..."
voms-proxy-info -exists  -valid 24:00 && echo "YES" || echo "NO"

myexit 0
