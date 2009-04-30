#!/bin/sh

# Test grid-cert-info with different options

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
    echo "    === test PASSED === "
  fi
   
  exit 0
}

function myecho()
{
  echo "#grid-cert-info-test# $1"
}

echo "    === Option test of grid-cert-info === "
echo ""
myecho "grid-cert-info without option ..."
grid-cert-info || myexit 1

echo ""
myecho "grid-cert-info -all ..."
grid-cert-info -all || myexit 1

echo ""
myecho "grid-cert-info -subject ..."
grid-cert-info -subject || myexit 1

echo ""
myecho "grid-cert-info -issuer ..."
grid-cert-info -issuer || myexit 1

echo ""
myecho "grid-cert-info -startdate ..."
grid-cert-info -startdate || myexit 1

echo ""
myecho "grid-cert-info -enddate ..."
grid-cert-info -enddate || myexit 1

myexit 0
