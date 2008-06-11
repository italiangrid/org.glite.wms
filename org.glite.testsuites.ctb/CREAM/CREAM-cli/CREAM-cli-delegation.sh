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

FAILED=0

my_echo "TEST 0: delegate a proxy specifying CREAM URL and delegation ID:"

PROXY_ID=`new_delegation_id`
run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-delegate-proxy -e $ENDPOINT $PROXY_ID
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  success
fi

my_echo "TEST 1: re-delegate the proxy specified in TEST 0:"

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-delegate-proxy -e $ENDPOINT $PROXY_ID
RESULT=`echo ${COM_OUTPUT} | grep "Delegation ID '$PROXY_ID' already exists"`
if [ -z "$RESULT" ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  success
fi

my_echo "TEST 2: delegate a proxy and append the output to the existing file ${LOGFILE}:";
echo "#HEADER#" > ${LOGFILE} || exit_failure "Cannot open ${LOGFILE}";
run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-delegate-proxy \
--logfile ${LOGFILE} -e $ENDPOINT `new_delegation_id`
RESULT=`grep "#HEADER#" ${LOGFILE}`
if [ -z "$RESULT" ]; then
  failure "File ${LOGFILE} has been overwrite"
  ((FAILED++)) # continue
else
  RESULT=`grep -P "INFO|ERROR|WARN|NOTICE" ${LOGFILE}`
  if [ -z "$RESULT" ]; then
    failure "Cannot log on file ${LOGFILE}"
    ((FAILED++)) # continue
  else
    success
  fi
fi

my_echo "TEST 3: delegate a proxy specifying a client configuration file:";
mkdir ${MYTMPDIR}/delegate_log_dir || exit_failure "Cannot create ${MYTMPDIR}/delegate_log_dir";
printf "[
DELEGATE_LOG_DIR=\"${MYTMPDIR}/delegate_log_dir\";
CREAMDELEGATION_URL_PREFIX=\"https://\";
CREAMDELEGATION_URL_POSTFIX=\"ce-cream/services/gridsite-delegation\";
]
" > ${MYTMPDIR}/delegate.conf
run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-delegate-proxy \
--debug --conf ${MYTMPDIR}/delegate.conf -e $ENDPOINT `new_delegation_id`
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  RESULT=`ls ${MYTMPDIR}/delegate_log_dir/* | grep glite-ce-delegate-proxy_CREAM | wc -l 2>/dev/null`
  if [ $RESULT == "0" ]; then
    failure "Cannot find debug log file"
    ((FAILED++)) # continue
  else
    success
  fi
fi

if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 4 differents tests"
else
  exit_success
fi

