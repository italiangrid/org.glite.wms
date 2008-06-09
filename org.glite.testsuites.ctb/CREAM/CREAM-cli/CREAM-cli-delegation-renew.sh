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

my_echo "TEST 0: renew a proxy specifying CREAM URL and delegation ID:"

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-delegate-proxy -e $ENDPOINT DelegateId_$$_0
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-proxy-renew -e $ENDPOINT DelegateId_$$_0
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
else
  success
fi

my_echo "TEST 1: try to renew a missing proxy:"
run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-proxy-renew -e $ENDPOINT DelegateId_$$_1
RESULT=`echo ${COM_OUTPUT} | grep "delegation ID was not delegated"`
if [ -z "$RESULT" ]; then
  exit_failure ${COM_OUTPUT}
else
  success
fi

my_echo "TEST 2: renew a proxy and append the output to the existing file ${LOGFILE}:";
echo "#HEADER#" > ${LOGFILE} || exit_failure "Cannot open ${LOGFILE}";
run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-proxy-renew \
--logfile ${LOGFILE} -e $ENDPOINT DelegateId_$$_0
RESULT=`grep "#HEADER#" ${LOGFILE}`
if [ -z "$RESULT" ]; then
  exit_failure "File ${LOGFILE} has been overwrite"
else
  RESULT=`grep -P "NOTICE|ERROR|WARNING" ${LOGFILE}`
  if [ -z "$RESULT" ]; then
    exit_failure "Cannot log on file ${LOGFILE}"  
  else
    success
  fi
fi

my_echo "TEST 3: renew a proxy specifying a client configuration file:";
mkdir ${MYTMPDIR}/renew_log_dir || exit_failure "Cannot create ${MYTMPDIR}/renew_log_dir";
printf "[
PROXYRENEW_LOG_DIR=\"${MYTMPDIR}/renew_log_dir\";
CREAMDELEGATION_URL_PREFIX=\"https://\";
CREAMDELEGATION_URL_POSTFIX=\"ce-cream/services/gridsite-delegation\";
]
" > ${MYTMPDIR}/delegate.conf
run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-proxy-renew \
--debug --conf ${MYTMPDIR}/delegate.conf -e $ENDPOINT DelegateId_$$_0
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
else
  RESULT=`ls ${MYTMPDIR}/renew_log_dir/* | grep glite-ce-proxy-renew_CREAM | wc -l 2>/dev/null`
  if [ $RESULT == "0" ]; then
    exit_failure "Cannot find debug log file"
  else
    success
  fi
fi


exit_success
