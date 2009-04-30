#!/bin/sh

# Test grid-proxy-init/info/destroy chain.
# The following options of grid-proxy-init are used: -verify -debug -limited -valid -bits -out

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
  echo "#grid-proxy test# $1"
}

echo ""
echo "    === grid-proxy-init/info/destroy test === "

TMPPROXY=/tmp/proxy_`id  -u`

myecho "initializing new proxy, output to file $TMPPROXY ..."
grid-proxy-init -verify -debug -limited -valid 1:00 -bits 1024 -out $TMPPROXY
if [ $? -ne 0 ]; then
  myecho "ERROR: could not create proxy"
  myexit 1
fi

echo ""
myecho "trying grid-proxy-info ... "
grid-proxy-info -file $TMPPROXY
if [ $? -ne 0 ]; then
  myecho "ERROR: could not get grid proxy info"
  myexit 1
fi

echo ""
myecho "destroying proxy with grid-proxy-destroy ..."
grid-proxy-destroy $TMPPROXY
if [ $? -ne 0 ]; then
  myecho "ERROR: could not destroy proxy"
  myexit 1
fi

echo ""
myecho "checking if it is destroyed ..."
grid-proxy-info -file $TMPPROXY
if [ $? -eq 0 ]; then  
 myecho "ERROR: proxy has not been destroyed"
 myexit 1
fi
myecho "Yes, proxy has been successfully destroyed"

echo ""
myecho "trying grid-proxy-destroy when proxy is not there ..."
grid-proxy-destroy $TMPPROXY
myecho "grid-proxy-destroy exit code: $?"
if [ $? -eq 0 ]; then
  myecho "destroying no proxy finished successfully"
#  myexit 1
else
  myecho "destroying no proxy failed, as required"
fi

myexit 0
