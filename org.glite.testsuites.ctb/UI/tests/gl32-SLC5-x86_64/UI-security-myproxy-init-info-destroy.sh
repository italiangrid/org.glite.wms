#!/bin/sh

# Basic test of the myproxy-init/info/destroy command chain

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

function exit_failure() {
  echo ""
  echo " *** something went wrong *** "
  echo " please make sure have a valid grid/voms proxy"
  myproxy-destroy
  echo " *** test NOT passed *** "
  exit $1
}

function myecho()
{
  echo "# MyProxy test # $1"
}

echo ""
echo "   === MyProxy test === "

TMPPROXY=/tmp/delegation_`id -u`

echo ""
myecho "initializing new proxy with myproxy"
echo ${PASS} | myproxy-init --cred_lifetime 1 --proxy_lifetime 1 -n --stdin_pass
if [ $? -ne 0 ]; then
  myecho "ERROR: could not create long-time proxy"
  exit_failure 1
fi

echo ""
myecho "trying myproxy-info ... "
myproxy-info
if [ $? -ne 0 ]; then
  myecho "ERROR: myproxy-info failed"
  exit_failure 1
fi

echo ""
myecho "destroying long time proxy with myproxy-destroy ..."
myproxy-destroy
if [ $? -ne 0 ]; then
  myecho "ERROR: myproxy-destroy failed"
  exit_failure 1
fi

echo ""
myecho "checking if it is destroied ..."
myproxy-info
if [ $? -eq 0 ]; then  
 myecho "ERROR: MyProxy proxy has not been destroyed"
 exit_failure 1
fi
myecho "Yes, the proxy has been successfully destroyed"

echo ""
myecho "trying myproxy-destroy when myproxy already destroyed ..."
myproxy-destroy
if [ $? -eq 0 ]; then
  myecho "ERROR: destroying no proxy should result in a failure"
  exit_failure 1
fi
myecho "destroying no proxy failed, as required"

echo ""
myecho "initializing new proxy with a false server"
MYPROXY_SERVER="no.such.server" 
echo ${PASS} | myproxy-init --cred_lifetime 1 --proxy_lifetime 1 -n --stdin_pass
if [ $? -eq 0 ]; then
  myecho "ERROR: Using a false server succeeded"
  exit_failure 1
fi
myecho "Using a false server results in a failure, as required"


echo ""
echo "   === test PASSED === " 
exit 0
