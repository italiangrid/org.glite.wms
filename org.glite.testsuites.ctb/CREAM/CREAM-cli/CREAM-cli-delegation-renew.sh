#!/bin/sh

###############################################################################
#
# Test glite-ce-proxy-renew command
#
# TEST 1: renew a proxy specifying CREAM URL and delegation ID
# TEST 2: try to renew a missing proxy
# TEST 3: check if the --conf option works
# TEST 4: save info into a logfile to check if --debug and --logfile options work
#
# Author: Paolo Andreetto <sa3-italia@mi.infn.it>
# Version: $Id:
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

FAILED=0

my_echo ""

TESTCOMMAND="${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-proxy-renew"

if [ ! -x ${TESTCOMMAND} ] ; then
  exit_failure "Command ${TESTCOMMAND} not exists, test could not be performed!"
fi

####

my_echo "TEST 1: renew a proxy specifying CREAM URL and delegation ID:"

PROXY_ID=`new_delegation_id`
run_command "${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-delegate-proxy -e $ENDPOINT $PROXY_ID"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  run_command "${TESTCOMMAND} --endpoint $ENDPOINT $PROXY_ID"
  if [ $? -ne 0 ]; then
    exit_failure "Command failed: ${COM_OUTPUT}"
  else
    success
  fi
fi

####

my_echo "TEST 2: try to renew a missing proxy:"
run_command "${TESTCOMMAND} --endpoint $ENDPOINT `new_delegation_id`"
if [ $? -ne 1 ]; then
  exit_failure "Command unexpected success:: ${COM_OUTPUT}"
fi
RESULT=`echo ${COM_OUTPUT} | grep "delegation ID was not delegated"`
if [ -z "$RESULT" ]; then
  failure "Error message is not as expected: ${COM_OUTPUT}"
  ((FAILED++)) # continue
else
  success
fi

####

my_echo "TEST 3: check if the --conf option works:";
mkdir ${MYTMPDIR}/renew_log_dir || exit_failure "Cannot create ${MYTMPDIR}/renew_log_dir";
printf "[
PROXYRENEW_LOG_DIR=\"${MYTMPDIR}/renew_log_dir\";
CREAMDELEGATION_URL_PREFIX=\"https://\";
CREAMDELEGATION_URL_POSTFIX=\"ce-cream/services/gridsite-delegation\";
]
" > ${MYTMPDIR}/delegate.conf
run_command "${TESTCOMMAND} --debug --conf ${MYTMPDIR}/delegate.conf \
--endpoint $ENDPOINT $PROXY_ID"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  RESULT=`ls ${MYTMPDIR}/renew_log_dir/* | grep glite-ce-proxy-renew_CREAM | wc -l 2>/dev/null`
  if [ $RESULT == "0" ]; then
    failure "Cannot find debug log file"
    ((FAILED++)) # continue
  else
    success
  fi
fi

####

my_echo "TEST 4: save info into a logfile to check if --debug and --logfile options work:";
echo "#HEADER#" > ${LOGFILE} || exit_failure "Cannot open ${LOGFILE}";
run_command "${TESTCOMMAND} --logfile ${LOGFILE} --endpoint $ENDPOINT $PROXY_ID"
RESULT=`grep "#HEADER#" ${LOGFILE}`
if [ -z "$RESULT" ]; then
  failure "File ${LOGFILE} has been overwritten"
  ((FAILED++)) # continue
else
  RESULT=`grep -o -P "NOTICE|ERROR|WARNING" ${LOGFILE}`
  if [ -z "$RESULT" ]; then
    failure "Cannot log on file ${LOGFILE}"
    ((FAILED++)) # continue
  else
    success
  fi
fi

#### FINISHED

if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 4 differents tests"
else
  exit_success
fi
