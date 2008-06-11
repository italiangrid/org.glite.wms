#!/bin/sh

###############################################################################
#
# A basic job hold operation
#
# Features: This test performs several suspend calls on the server.
#
# Author: Paolo Andreetto <sa3-italia@mi.infn.it>
# Version: $Id:
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

FAILED=0

my_echo "TEST 0: submit and then suspend a running job:"

wait_until_job_runs

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-suspend -N $JOBID
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  success
fi


my_echo "TEST 1: submit and then suspend jobs with --all option:"

wait_until_job_runs

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-suspend -N --all -e $ENDPOINT
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  success
fi

my_echo "TEST 2: try to suspend a terminated job:"

wait_until_job_finishes

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-suspend -N $JOBID
RESULT=`echo ${COM_OUTPUT} | grep "status not compatible with the JOB_SUSPEND command"`
if [ -z "$RESULT" ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  success
fi

my_echo "TEST 3: submit and suspend a job and append the output to the existing file ${LOGFILE}:";
echo "#HEADER#" > ${LOGFILE} || exit_failure "Cannot open ${LOGFILE}";

wait_until_job_runs

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-suspend \
--logfile ${LOGFILE} -d -N $JOBID
RESULT=`grep "#HEADER#" ${LOGFILE}`
if [ -z "$RESULT" ]; then
  failure "File ${LOGFILE} has been overwrite"
  ((FAILED++)) # continue
else
  RESULT=`grep -o -P "INFO|ERROR|WARN|NOTICE" ${LOGFILE}`
  if [ -z "$RESULT" ]; then
    failure "Cannot log on file ${LOGFILE}"
    ((FAILED++)) # continue
  else
    success
  fi
fi

my_echo "TEST 4: submit and suspend a job specifying a client configuration file:"
mkdir ${MYTMPDIR}/suspend_log_dir || exit_failure "Cannot create ${MYTMPDIR}/suspend_log_dir";
printf "[
SUSPEND_LOG_DIR=\"${MYTMPDIR}/suspend_log_dir\";
CREAM_URL_PREFIX=\"https://\";
CREAM_URL_POSTFIX=\"ce-cream/services/CREAM2\";
]
" > ${MYTMPDIR}/suspend.conf
run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-suspend --debug \
--conf ${MYTMPDIR}/suspend.conf -N $JOBID
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  RESULT=`ls ${MYTMPDIR}/suspend_log_dir/* | grep glite-ce-job-suspend_CREAM | wc -l 2>/dev/null`
  if [ $RESULT == "0" ]; then
    failure "Cannot find debug log file"
    ((FAILED++)) # continue
  else
    success
  fi
fi


if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 3 differents tests"
else
  exit_success
fi
