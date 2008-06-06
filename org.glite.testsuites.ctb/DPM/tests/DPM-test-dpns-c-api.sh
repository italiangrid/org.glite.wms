#!/bin/bash

. ./helper.sh

if [ "x$1" = "x" ]; then
 if [ ! "x$DPM_HOST" = "x" ]; then
   node=$DPM_HOST
 else
  echo_failure;
  echo "  No target node defined ! Exiting."
  echo "  Usage: $0 <target node> <vo>"
  exit 1
 fi
else
 node=$1
fi

if [ "x$2" = "x" ]; then
 echo "   No vo specified ! Exiting."
 echo "   Usage: $0 <target node> <vo>"
 exit 1
else
 vo=$2
fi

echo "Target node is $node. `date`"
echo "Launching DPNS C API tests:"

rm -f DPM-test-dpns-c-api.exe
gcc -I /opt/lcg/include/dpm/ -L/opt/lcg/lib/ -ldpm -lgfal DPM-test-dpns-c-api.c  -o DPM-test-dpns-c-api.exe

if [ $? -eq 0 ] && [ -f DPM-test-dpns-c-api.exe ]; then
 echo_success
else
 echo_failure
 exit 1
fi
echo "  Compilation of the c program:"

mydomain=`echo $node | cut -d "." -f 2-`
mypath=/dpm/$mydomain/home/$vo/dpnscapitest$RANDOM

export DPM_HOST=$node
export DPNS_HOST=$node

./DPM-test-dpns-c-api.exe $mypath

