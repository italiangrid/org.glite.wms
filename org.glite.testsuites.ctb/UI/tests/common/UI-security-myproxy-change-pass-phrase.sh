#!/bin/sh
#
# Basic test of the myproxy-init, myproxy-change-pass-phrase, and myproxy-destroy
# This script prompts the user for his credential password, but generates myproxy password(s) internally
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

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

echo ""

# ... ask user for GRID password (-s = silent mode; -r = treat backslashes as normal character; -p = prompt text)

#read -s -r -p "Enter GRID pass phrase: "

#REPLY="$REPLY" # but if RANDOM unset?
REPLY=$PASS
PD=${RANDOM}${RANDOM}${RANDOM}$$

# ... create myproxy proxy

myecho "creating myproxy credential with myproxy-init ..."
myproxy-init --cred_lifetime 1 --proxy_lifetime 1 --stdin_pass <<myinput
$REPLY
$PD
myinput

REPLY=""

if [ $? -ne 0 ]; then
  myecho "ERROR: could not create long-time proxy"
  exit_failure 1
fi

NEWPD=${RANDOM}${RANDOM}${RANDOM}$$

# ... change password

echo ""
myecho "changing myproxy passphrase with myproxy-change-pass-phrase ..."
myproxy-change-pass-phrase --stdin_pass <<konec
$PD
$NEWPD
konec

if [ $? -ne 0 ]; then
  myecho "ERROR: could not create long-time proxy"
  exit_failure 1
fi

# ... destroy the proxy

echo ""
myecho "destroying long time proxy with myproxy-destroy ..."
myproxy-destroy
if [ $? -ne 0 ]; then
  myecho "ERROR: myproxy-destroy failed"
  exit_failure 1
fi

echo ""
echo "   === test PASSED === " 
exit 0
