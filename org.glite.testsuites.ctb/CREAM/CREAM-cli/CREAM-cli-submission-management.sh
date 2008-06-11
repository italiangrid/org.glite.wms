#!/bin/sh

###############################################################################
#
# A basic glite-ce-allowed submission test.
#
# Features: The test will fail when one of the tested commands fails,
# but not when the job itself finishes with a failure or aborted status.
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

####

my_echo "TEST 1: check if submissions to CE are allowed:"

run_command glite-ce-allowed-submission -n $ENDPOINT
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
else
  success
fi

####

my_echo "TEST 2: check the --conf option:"

mkdir ${MYTMPDIR}/allowsub_log_dir || exit_failure "Cannot create ${MYTMPDIR}/allowsub_log_dir";
printf "[
ALLOWEDSUB_LOG_DIR=\"${MYTMPDIR}/allowsub_log_dir\";
]
" > ${MYTMPDIR}/allowsub.conf
run_command glite-ce-allowed-submission --debug --conf ${MYTMPDIR}/allowsub.conf $ENDPOINT
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
else
  RESULT=`ls ${MYTMPDIR}/allowsub_log_dir/* | grep glite-ce-allowed-submission_CREAM | wc -l 2>/dev/null`
  if [ $RESULT == "0" ]; then
    failure "Cannot find debug log file"
    ((FAILED++))
  else
    success
  fi
fi

####

my_echo "TEST 3: save info into a logfile (-d --logfile):"

echo "#HEADER#" > ${LOGFILE} || exit_failure "Cannot open ${LOGFILE}";

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-allowed-submission -d --logfile ${LOGFILE} $ENDPOINT
RESULT=`grep "#HEADER#" ${LOGFILE}`
if [ -z "$RESULT" ]; then
  exit_failure "File ${LOGFILE} has been overwrite"
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
