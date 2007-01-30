#!/bin/bash

# Test grid-proxy-info with different options

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
  echo "#grid-proxy-info test# $1"
}

echo "    === Option test of grid-proxy-info === "
echo ""
myecho "grid-proxy-info without options ..."
grid-proxy-info || myexit 1

echo ""
myecho "grid-proxy-info -all ..."
grid-proxy-info -all || myexit 1

echo ""
myecho "grid-proxy-info -edebug ..."
grid-proxy-info -debug || myexit 1

echo ""
myecho "grid-proxy-info -text ..."
grid-proxy-info -text || myexit 1

echo ""
myecho "grid-proxy-info -subject ..."
grid-proxy-info -subject || myexit 1

echo ""
myecho "grid-proxy-info -issuer ..."
grid-cert-info -issuer || myexit 1

echo ""
myecho "grid-proxy-info -identity ..."
grid-proxy-info -identity || myexit 1

echo ""
myecho "grid-proxy-info -type ..."
grid-proxy-info -type || myexit 1

echo ""
myecho "grid-proxy-info -timeleft ..."
grid-proxy-info -timeleft || myexit 1

echo ""
myecho "grid-proxy-info -strength ..."
grid-proxy-info -strength || myexit 1

echo ""
myecho "grid-proxy-info -path ..."
grid-proxy-info -path || myexit 1

echo ""
myecho "grid-proxy-info -exists -bits 256 ..."
grid-proxy-info -exists -bits 256 && echo "YES" || echo "NO"

echo ""
myecho "grid-proxy-info -exists -bits 512 ..."
grid-proxy-info -exists -bits 512 && echo "YES" || echo "NO"

echo ""
myecho "grid-proxy-info -exists -bits 1024 ..."
grid-proxy-info -exists -bits 1024 && echo "YES" || echo "NO"

echo ""
myecho "grid-proxy-info -exists -valid 1 ..."
grid-proxy-info -exists  -valid 1:00 && echo "YES" || echo "NO"

echo ""
myecho "grid-proxy-info -exists -valid 3 ..."
grid-proxy-info -exists  -valid 3:00 && echo "YES" || echo "NO"

echo ""
myecho "grid-proxy-info -exists -valid 10 ..."
grid-proxy-info -exists  -valid 10:00 && echo "YES" || echo "NO"

echo ""
myecho "grid-proxy-info -exists -valid 24 ..."
grid-proxy-info -exists  -valid 24:00 && echo "YES" || echo "NO"

myexit 0
