#!/bin/sh

###############################################################################
#
# A basic proxy delegation test.
#
# Features: This test performs several delegation calls on the server.
#
# Author: Paolo Andreetto <sa3-italia@mi.infn.it>
# Version: $Id:
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

my_echo "TEST 0: delegate a proxy specifying CREAM URL and delegation ID:"

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-delegate-proxy -e $ENDPOINT DelegateId_$$_0
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
else
  success
fi

my_echo "TEST 1: re-delegate the proxy specified in TEST 0:"

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-delegate-proxy -e $ENDPOINT DelegateId_$$_0
RESULT=`echo ${COM_OUTPUT} | grep "Delegation ID 'DelegateId_$$_0' already exists"`
if [ -z "$RESULT" ]; then
  exit_failure ${COM_OUTPUT}
else
  success
fi

