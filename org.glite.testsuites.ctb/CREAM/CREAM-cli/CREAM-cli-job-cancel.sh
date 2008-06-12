#!/bin/sh

###############################################################################
#
# A basic job interruption operation
#
# Features: This test performs several cancel calls on the server.
#
# Author: Paolo Andreetto <sa3-italia@mi.infn.it>
# Version: $Id:
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

FAILED=0

my_echo "TEST 0: submit and then cancel a running job:"

wait_until_job_runs
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-cancel -N $JOBID
  if [ $? -ne 0 ]; then
    failure ${COM_OUTPUT}
    ((FAILED++)) # continue
  else
    success
  fi
fi

my_echo "TEST 1: submit and then cancel jobs with --all option:"

wait_until_job_runs
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-cancel -N --all -e $ENDPOINT
  if [ $? -ne 0 ]; then
    failure ${COM_OUTPUT}
    ((FAILED++)) # continue
  else
    success
  fi
fi

my_echo "TEST 2: try to cancel a terminated job:"

wait_until_job_finishes

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-cancel -N $JOBID
RESULT=`echo ${COM_OUTPUT} | grep "status not compatible with the JOB_CANCEL command"`
if [ -z "$RESULT" ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  success
fi

my_echo "TEST 3: submit and cancel a job and append the output to the existing file ${LOGFILE}:";
echo "#HEADER#" > ${LOGFILE} || exit_failure "Cannot open ${LOGFILE}";

wait_until_job_runs
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-cancel --logfile ${LOGFILE} -d -N $JOBID
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
fi

my_echo "TEST 4: submit and cancel a job specifying a client configuration file:"
mkdir ${MYTMPDIR}/cancel_log_dir || exit_failure "Cannot create ${MYTMPDIR}/cancel_log_dir";
printf "[
CANCEL_LOG_DIR=\"${MYTMPDIR}/cancel_log_dir\";
CREAM_URL_PREFIX=\"https://\";
CREAM_URL_POSTFIX=\"ce-cream/services/CREAM2\";
]
" > ${MYTMPDIR}/cancel.conf
run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-cancel --debug \
--conf ${MYTMPDIR}/cancel.conf -N $JOBID
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  RESULT=`ls ${MYTMPDIR}/cancel_log_dir/* | grep glite-ce-job-cancel_CREAM | wc -l 2>/dev/null`
  if [ $RESULT == "0" ]; then
    failure "Cannot find debug log file"
    ((FAILED++)) # continue
  else
    success
  fi
fi






if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 5 differents tests"
else
  exit_success
fi

