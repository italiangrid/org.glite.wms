#!/bin/sh

###############################################################################
#
# Test glite-ce-job-suspend command:
#
# TEST 1: submit and then suspend a running job (it works only with some batch systems)
# TEST 2: check if --all option works
# TEST 3: try to suspend a terminated job
# TEST 4: save info into a logfile to check if --debug and --logfile options work
# TEST 5: check if the --conf option works 
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

TESTCOMMAND="${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-suspend"

if [ ! -x ${TESTCOMMAND} ] ; then
  exit_failure "Command ${TESTCOMMAND} not exists, test could not be performed!"
fi

####

my_echo "TEST 1: submit and then suspend a running job (it works only with some batch systems):"

wait_until_job_runs

run_command "${TESTCOMMAND} --noint $JOBID"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
  ((FAILED++)) # continue
else
  run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-suspend -N $JOBID
  if [ $? -ne 0 ]; then
    failure ${COM_OUTPUT}
    ((FAILED++)) # continue
  else
    sleep 10
    extract_status $JOBID
    if [ "$JOBSTATUS" == "HELD" ]; then
      success
    else
      failure "The job is not suspended, status is $JOBSTATUS"
      ((FAILED++)) # continue
    fi
  fi
fi

####

my_echo "TEST 2: check if --all option works:"

wait_until_job_runs

run_command "${TESTCOMMAND} --noint --all --endpoint $ENDPOINT"
if [ $? -ne 0 ]; then
  failure "Command failed: ${COM_OUTPUT}"
else
  run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-suspend -N --all -e $ENDPOINT
  if [ $? -ne 0 ]; then
    failure ${COM_OUTPUT}
    ((FAILED++)) # continue
  else
    sleep 10
    extract_status $JOBID
    if [ "$JOBSTATUS" == "HELD" ]; then
      success
    else
      failure "The job is not suspended, status is $JOBSTATUS"
      ((FAILED++)) # continue
    fi
  fi
fi

####

my_echo "TEST 3: try to suspend a terminated job:"

wait_until_job_finishes

run_command "${TESTCOMMAND} --noint $JOBID"
RESULT=`echo ${COM_OUTPUT} | grep "status not compatible with the JOB_SUSPEND command"`
if [ -z "$RESULT" ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  success
fi

####

my_echo "TEST 4: save info into a logfile to check if --debug and --logfile options work:";
echo "#HEADER#" > ${LOGFILE} || exit_failure "Cannot open ${LOGFILE}";

wait_until_job_runs

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


my_echo "TEST 5: check if the --conf option works:"
mkdir ${MYTMPDIR}/suspend_log_dir || exit_failure "Cannot create ${MYTMPDIR}/suspend_log_dir";
printf "[
SUSPEND_LOG_DIR=\"${MYTMPDIR}/suspend_log_dir\";
CREAM_URL_PREFIX=\"https://\";
CREAM_URL_POSTFIX=\"ce-cream/services/CREAM2\";
]
" > ${MYTMPDIR}/suspend.conf
run_command "${TESTCOMMAND}  --debug --conf ${MYTMPDIR}/suspend.conf --noint $JOBID"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  RESULT=`ls ${MYTMPDIR}/suspend_log_dir/* | grep glite-ce-job-suspend_CREAM | wc -l 2>/dev/null`
  if [ $RESULT == "0" ]; then
    failure "Cannot find debug log file"
    ((FAILED++)) # continue
  else
    success
  fi
fi

####

if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 5 differents tests"
else
  exit_success
fi
