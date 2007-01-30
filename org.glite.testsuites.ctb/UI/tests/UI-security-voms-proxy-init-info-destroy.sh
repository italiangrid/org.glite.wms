#!/bin/bash

# Test voms-proxy-init/info/destroy chain.
# The following options of voms-proxy-init are used: -verify -debug -limited -valid -bits -out

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
  echo "#voms-proxy test# $1"
}

echo ""
echo "    === voms-proxy test === "

TMPPROXY=/tmp/proxy_`id  -u`

echo ""
myecho "initializing new proxy, output to file $TMPPROXY ..."
voms-proxy-init  -verify -debug -limited -valid 1:00 -bits 1024 -out $TMPPROXY
if [ $? -ne 0 ]; then
  myecho "ERROR: could not create proxy"
  myexit 1
fi

echo ""
myecho "trying voms-proxy-info ... "
voms-proxy-info -file $TMPPROXY
if [ $? -ne 0 ]; then
  myecho "ERROR: could not get voms proxy info"
  myexit 1
fi

echo ""
myecho "destroying proxy with voms-proxy-destroy ..."
voms-proxy-destroy -file $TMPPROXY
if [ $? -ne 0 ]; then
  myecho "ERROR: could not destroy proxy"
  myexit 1
fi

echo ""
myecho "checking if it is destroied ..."
voms-proxy-info -file $TMPPROXY
if [ $? -eq 0 ]; then  
 myecho "ERROR: proxy has not been destroyed"
 myexit 1
fi
myecho "Yes, proxy has been successfully destroyed"

echo ""
myecho "trying voms-proxy-destroy when proxy is not there ..."
voms-proxy-destroy -file $TMPPROXY
if [ $? -eq 0 ]; then
  myecho "ERROR: destroying no proxy should result in a failure"
  myexit 1
fi
myecho "destroying no proxy failed, as required"

myexit 0
