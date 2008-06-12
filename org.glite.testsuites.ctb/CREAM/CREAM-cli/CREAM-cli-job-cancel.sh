#!/bin/sh

###############################################################################
#
# Test glite-ce-job-cancel command:
#
# TEST 1: submit and then cancel a running job
# TEST 2: try to cancel a terminated job
# TEST 3: check if the --all option works
# TEST 4: check the requirements of the --all option (3 cases)
# TEST 5: check if the --conf option works
# TEST 6: save info into a logfile to check if --debug and --logfile options work 
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

TESTCOMMAND="${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-cancel"

if [ ! -x ${TESTCOMMAND} ] ; then
  exit_failure "Command ${TESTCOMMAND} not exists, test could not be performed!"
fi

####

my_echo "TEST 1: submit and then cancel a running job:"

wait_until_job_runs

run_command "${TESTCOMMAND} --noint $JOBID"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  success
fi

####

my_echo "TEST 2: try to cancel a terminated job:"

wait_until_job_finishes

run_command "${TESTCOMMAND} --noint $JOBID"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi
RESULT=`echo ${COM_OUTPUT} | grep "status not compatible with the JOB_CANCEL command"`
if [ -z "$RESULT" ]; then
  failure "Error message is not as expected: ${COM_OUTPUT}"
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

####

my_echo "TEST 3: check if the --all option works:"

wait_until_job_runs

run_command "${TESTCOMMAND} --noint --all --endpoint $ENDPOINT"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  success
fi

####

my_echo "TEST 4: check the requirements of the --all option (3 cases):"

# -a and -i are not compatible
run_command "${TESTCOMMAND} --nomsg  --all --input $MYTMPDIR/jobid"
if [ $? -ne 1 ]; then
  exit_failure "Command unexpected success: ${COM_OUTPUT}"
fi

# check the error message
TMP=`echo ${COM_OUTPUT} | grep "all and --input or --all and specification of JobID(s) as argument are exclusive"`
if [ $? -ne 0 ]; then
  failure " The ouput of the command is: ${COM_OUTPUT}"
  ((FAILED++)) # continue
else
  success
fi

# -a requires -e
run_command "${TESTCOMMAND} --nomsg  --all"
if [ $? -ne 1 ]; then
  exit_failure "Command unexpected success: ${COM_OUTPUT}"
fi

# check the error message
TMP=`echo ${COM_OUTPUT} | grep "Option --all requires the specification of the endpoint"`

if [ $? -ne 0 ]; then
  failure " The ouput of the command is: ${COM_OUTPUT}"
  ((FAILED++)) # continue
else
  success
fi

# -a and JOBID are not compatible
run_command "${TESTCOMMAND} --nomsg  --all $JOBID"
if [ $? -ne 1 ]; then
  exit_failure "Command unexpected success: ${COM_OUTPUT}"
fi

# check the error message
TMP=`echo ${COM_OUTPUT} | grep "all and --input or --all and specification of JobID(s) as argument are exclusive"`
if [ $? -ne 0 ]; then
  failure " The ouput of the command is: ${COM_OUTPUT}"
  ((FAILED++)) # continue
else
  success
fi

####

my_echo "TEST 5: check if the --conf option works:"
mkdir ${MYTMPDIR}/cancel_log_dir || exit_failure "Cannot create ${MYTMPDIR}/cancel_log_dir";
printf "[
CANCEL_LOG_DIR=\"${MYTMPDIR}/cancel_log_dir\";
CREAM_URL_PREFIX=\"https://\";
CREAM_URL_POSTFIX=\"ce-cream/services/CREAM2\";
]
" > ${MYTMPDIR}/cancel.conf
run_command "${TESTCOMMAND} --debug --conf ${MYTMPDIR}/cancel.conf --noint $JOBID"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  RESULT=`ls ${MYTMPDIR}/cancel_log_dir/* | grep glite-ce-job-cancel_CREAM | wc -l 2>/dev/null`
  if [ $RESULT == "0" ]; then
    failure "Cannot find debug log file"
    ((FAILED++)) # continue
  else
    success
  fi
fi

####

my_echo "TEST 6: save info into a logfile to check if --debug and --logfile options work:";
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

#### FINISHED

if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 8 differents tests"
else
  exit_success
fi

