#!/bin/sh

###############################################################################
#
# Test glite-ce-allowed-submission command:
#
# TEST 1: check if the submissions to the CE are allowed
# TEST 2: check if the --conf option works
# TEST 3: save info into a logfile to check if --debug and --logfile options work
#
# Author: Alessio Gianelle <sa3-italia@mi.infn.it>
# Version: $Id:
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

FAILED=0

my_echo ""

TESTCOMMAND="${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-allowed-submission"

if [ ! -x ${TESTCOMMAND} ] ; then
  exit_failure "Command ${TESTCOMMAND} not exists, test could not be performed!"
fi

####

my_echo "TEST 1: check if the submissions to the CE are allowed:"

run_command "${TESTCOMMAND} --nomsg $ENDPOINT"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  success
fi

####

my_echo "TEST 2: check if the --conf option works:"

mkdir ${MYTMPDIR}/allowsub_log_dir || exit_failure "Cannot create ${MYTMPDIR}/allowsub_log_dir";
printf "[
ALLOWEDSUB_LOG_DIR=\"${MYTMPDIR}/allowsub_log_dir\";
]
" > ${MYTMPDIR}/allowsub.conf
run_command "${TESTCOMMAND} --debug --conf ${MYTMPDIR}/allowsub.conf $ENDPOINT"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  RESULT=`ls ${MYTMPDIR}/allowsub_log_dir/* | grep glite-ce-allowed-submission_CREAM | wc -l 2>/dev/null`
  if [ $RESULT == "0" ]; then
    failure "Cannot find debug log file"
    ((FAILED++)) # continue
  else
    success
  fi
fi

####

my_echo "TEST 3: save info into a logfile to check if --debug and --logfile options work:"

echo "#HEADER#" > ${LOGFILE} || exit_failure "Cannot open ${LOGFILE}";

run_command "${TESTCOMMAND} --debug --logfile ${LOGFILE} $ENDPOINT"
RESULT=`grep "#HEADER#" ${LOGFILE}`
if [ -z "$RESULT" ]; then
  failure "File ${LOGFILE} has been overwritten"
	((FAILED++)) # continue
else
  RESULT=`grep -P "INFO|ERROR|WARN" ${LOGFILE}`
  if [ -z "$RESULT" ]; then
    failure "Cannot log on file ${LOGFILE}"
    ((FAILED++)) # continue
  else
    success
  fi
fi

####

#### FINISHED

if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 3 differents tests"
else
  exit_success
fi
