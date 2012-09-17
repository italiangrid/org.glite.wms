#!/bin/sh

###############################################################################
#
# Test glite-ce-job-resume command:
#
# TEST 1: submit, suspend and then resume a running job
# TEST 2: check if --all option works
# TEST 3: try to resume a running job
# TEST 4: save info into a logfile to check if --debug and --logfile options work
# TEST 5: check if the --conf option works 
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
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

run_command "${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-suspend --noint $JOBID"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

my_echo "Waiting for suspended job (10s)"
sleep 10
i=0
extract_status $JOBID
while [ "$JOBSTATUS" != "HELD" ] ; do
	if [ $i -ge $NUM_STATUS_RETRIEVALS ]; then
  	COM_OUTPUT="TIMEOUT reached! The job is not suspended, status is $JOBSTATUS"
		return 1
  fi
	sleep 5
	extract_status $JOBID
	((i++))
done	

return 0;
}

prepare $@

FAILED=0

my_echo ""

TESTCOMMAND="${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-resume"

if [ ! -x ${TESTCOMMAND} ] ; then
  exit_failure "Command ${TESTCOMMAND} not exists, test could not be performed!"
fi

####

my_echo "TEST 1: submit, suspend and then resume a running job:"

submit_and_suspend
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  run_command "${TESTCOMMAND} --noint $JOBID"
  if [ $? -ne 0 ]; then
    exit_failure "Command failed: ${COM_OUTPUT}"
  else
    success
  fi
fi

####

my_echo "TEST 2: check if --all option works:"

submit_and_suspend
if [ $? -ne 0 ]; then
	failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  run_command "${TESTCOMMAND} --noint --all --endpoint $ENDPOINT"
  if [ $? -ne 0 ]; then
		exit_failure "Command failed: ${COM_OUTPUT}"
  else
    success
  fi
fi

####

my_echo "TEST 3: try to resume a running job:"

wait_until_job_runs
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  run_command "${TESTCOMMAND} --noint $JOBID"
  RESULT=`echo ${COM_OUTPUT} | grep "status not compatible with the JOB_RESUME command"`
  if [ -z "$RESULT" ]; then
    failure "Error message is not as expected: ${COM_OUTPUT}"
    ((FAILED++)) # continue
  else
    success
  fi
fi

####

my_echo "TEST 4: save info into a logfile to check if --debug and --logfile options work:";
echo "#HEADER#" > ${LOGFILE} || exit_failure "Cannot open ${LOGFILE}";

submit_and_suspend
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
	((FAILED++)) # continue
else
  run_command "${TESTCOMMAND} --logfile ${LOGFILE} --debug --noint $JOBID"
  RESULT=`grep "#HEADER#" ${LOGFILE}`
  if [ -z "$RESULT" ]; then
    failure "File ${LOGFILE} has been overwritten"
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

####

my_echo "TEST 5: check if the --conf option works:"
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
  run_command "${TESTCOMMAND} --debug --conf ${MYTMPDIR}/resume.conf --noint $JOBID"
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

#### FINISHED

if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 5 differents tests"
else
  exit_success
fi
