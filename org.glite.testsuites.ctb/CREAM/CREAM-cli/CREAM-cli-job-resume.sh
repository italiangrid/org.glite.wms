#!/bin/sh

###############################################################################
#
# A basic job resume operation
#
# Features: This test performs several resume calls on the server.
#
# Author: Paolo Andreetto <sa3-italia@mi.infn.it>
# Version: $Id:
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

function submit_and_suspend(){

wait_until_job_runs
if [ $? -ne 0 ]; then
  return 1
fi

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-suspend -N $JOBID
if [ $? -ne 0 ]; then
  return 1
fi

my_echo "Waiting for suspended job (10s)"
sleep 10
extract_status $JOBID
if [ ! "$JOBSTATUS" == "HELD" ]; then
  COM_OUTPUT="The job is not suspended, status is $JOBSTATUS"
  return 1
fi

return 0;
}

prepare $@

FAILED=0

my_echo "TEST 0: submit, suspend and then resume a running job:"

submit_and_suspend
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-resume -N $JOBID
  if [ $? -ne 0 ]; then
    failure ${COM_OUTPUT}
    ((FAILED++)) # continue
  else
    success
  fi
fi

my_echo "TEST 1: submit, suspend and then resume jobs with --all option:"

submit_and_suspend
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-resume -N --all -e $ENDPOINT
  if [ $? -ne 0 ]; then
    failure ${COM_OUTPUT}
    ((FAILED++)) # continue
  else
    success
  fi
fi

my_echo "TEST 2: try to resume a running job:"

wait_until_job_runs
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-resume -N $JOBID
  RESULT=`echo ${COM_OUTPUT} | grep "status not compatible with the JOB_RESUME command"`
  if [ -z "$RESULT" ]; then
    failure ${COM_OUTPUT}
    ((FAILED++)) # continue
  else
    success
  fi
fi



my_echo "TEST 3: resume a suspended job and append the output to the existing file ${LOGFILE}:";
echo "#HEADER#" > ${LOGFILE} || exit_failure "Cannot open ${LOGFILE}";

submit_and_suspend
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-resume --logfile ${LOGFILE} -d -N $JOBID
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

my_echo "TEST 4: resume a suspended job specifying a client configuration file:"
mkdir ${MYTMPDIR}/resume_log_dir || exit_failure "Cannot create ${MYTMPDIR}/cancel_log_dir";
printf "[
RESUME_LOG_DIR=\"${MYTMPDIR}/resume_log_dir\";
CREAM_URL_PREFIX=\"https://\";
CREAM_URL_POSTFIX=\"ce-cream/services/CREAM2\";
]
" > ${MYTMPDIR}/resume.conf

submit_and_suspend
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-resume --debug \
  --conf ${MYTMPDIR}/resume.conf -N $JOBID
  if [ $? -ne 0 ]; then
    failure ${COM_OUTPUT}
    ((FAILED++)) # continue
  else
    RESULT=`ls ${MYTMPDIR}/resume_log_dir/* | grep glite-ce-job-resume_CREAM | wc -l 2>/dev/null`
    if [ $RESULT == "0" ]; then
      failure "Cannot find debug log file"
      ((FAILED++)) # continue
    else
      success
    fi
  fi
fi



if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 5 differents tests"
else
  exit_success
fi
