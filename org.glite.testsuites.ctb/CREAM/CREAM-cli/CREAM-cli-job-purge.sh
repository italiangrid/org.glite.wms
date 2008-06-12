#!/bin/sh

###############################################################################
#
# Test glite-ce-job-purge command:
#
# TEST 1: purge a finished job
# TEST 2: try to purge a running job
# TEST 3: try to purge again a previous purged job
# TEST 4: purge more finished jobs
# TEST 5: purge more jobs taking jobIDs from a file (--input option)
# TEST 6: check if the --all option works
# TEST 7: check the requirements of the --all option (3 cases)
# TEST 8: check if the --conf option works
# TEST 9: save info into a logfile to check if --debug and --logfile options work 
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

TESTCOMMAND="${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-purge"

if [ ! -x ${TESTCOMMAND} ] ; then
  exit_failure "Command ${TESTCOMMAND} not exists, test could not be performed!"
fi

####

my_echo "Submit some jobs to prepare the context of the tests"

i=0
n=5

while [ $i -lt $n ] ; do
  run_command "${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-submit -a -o $MYTMPDIR/jobid -r $CREAM $JDLFILE"
  if [ $? -ne 0 ]; then
    exit_failure "Command failed: ${COM_OUTPUT}"
  fi
  ((i++))
done

my_echo ""

####

my_echo "TEST 1: purge a finished job:"

wait_until_job_finishes

# purge the job
run_command "${TESTCOMMAND} --noint $JOBID"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
	success
fi

# save the jobid for test 3
PURGEDJOB=$JOBID

####

my_echo "TEST 2: try to purge a running job:"

wait_until_job_runs
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  # purge the job
  run_command "${TESTCOMMAND} --nomsg --noint $JOBID"
  if [ $? -ne 0 ]; then
    exit_failure "Cpmmand failed: ${COM_OUTPUT}"
  fi

  # check the error message
  RESULT=`echo ${COM_OUTPUT} | grep -P "the job has a status not compatible with the JOB_PURGE command"`
  if [ -z "$RESULT" ]; then
    failure ${COM_OUTPUT}
    ((FAILED++)) # continue
  else
    success
  fi
fi
####

my_echo "TEST 3: try to purge again a previous purged job:"

run_command "${TESTCOMMAND} --nomsg --noint $PURGEDJOB"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

# check the error message
RESULT=`echo ${COM_OUTPUT} | grep -P "job not found"`
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  success
fi

####

my_echo "TEST 4: purge more finished jobs:"

wait_until_job_finishes
JOB1=${JOBID}
wait_until_job_finishes
JOB2=${JOBID}

# purge the jobs
run_command "${TESTCOMMAND} --nomsg --noint ${JOB1} ${JOB2}"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  success
fi

####

my_echo "TEST 5: purge more jobs taking jobIDs from a file (--input option):"

# purge the jobs
run_command "${TESTCOMMAND} --nomsg --noint --input $MYTMPDIR/jobid"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  success
fi

####

my_echo "TEST 6: check if the --all option works:"

run_command "${TESTCOMMAND} --noint --nomsg --all --endpoint $ENDPOINT"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  success
fi

####

my_echo "TEST 7: check the requirements of the --all option (3 cases):"

# -a and -i are not compatible
run_command "${TESTCOMMAND} --nomsg --noint --all --input $MYTMPDIR/jobid"
if [ $? -ne 1 ]; then
  exit_failure "Command unexpectly successed: ${COM_OUTPUT}"
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
run_command "${TESTCOMMAND} --nomsg --noint --all"
if [ $? -ne 1 ]; then
  exit_failure "Command unexpectly successed: ${COM_OUTPUT}"
fi

# check the error message
TMP=`echo ${COM_OUTPUT} | grep "Option --all requires the specification of the endpoint (option --endpoint) to contact"`
if [ $? -ne 0 ]; then
  failure " The ouput of the command is: ${COM_OUTPUT}"
  ((FAILED++)) # continue
else
  success
fi

# extract a jobid
JI=`tail -1 $MYTMPDIR/jobid`

# -a and JOBID are not compatible
run_command "${TESTCOMMAND} --nomsg --noint --all $JI"
if [ $? -ne 1 ]; then
  exit_failure "Command unexpectly successed: ${COM_OUTPUT}"
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

my_echo "TEST 8: check if the --conf option works:"

mkdir ${MYTMPDIR}/purge_log_dir || exit_failure "Cannot create ${MYTMPDIR}/purge_log_dir";
printf "[
PURGE_LOG_DIR=\"${MYTMPDIR}/purge_log_dir\";
]
" > ${MYTMPDIR}/purge.conf
run_command "${TESTCOMMAND} --noint --debug --conf ${MYTMPDIR}/purge.conf -i $MYTMPDIR/jobid"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  RESULT=`ls ${MYTMPDIR}/purge_log_dir/* | grep glite-ce-job-purge_CREAM | wc -l 2>/dev/null`
  if [ $RESULT == "0" ]; then
    failure "Cannot find debug log file"
    ((FAILED++))
  else
    success
  fi
fi

####

my_echo "TEST 9: save info into a logfile to check if --debug and --logfile options work:"

echo "#HEADER#" > ${LOGFILE} || exit_failure "Cannot open ${LOGFILE}";

run_command "${TESTCOMMAND} --noint --debug --logfile ${LOGFILE} -i $MYTMPDIR/jobid"
RESULT=`grep "#HEADER#" ${LOGFILE}`
if [ -z "$RESULT" ]; then
  failure "File ${LOGFILE} has been overwrite"
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
  exit_failure "$FAILED test(s) failed on 11 differents tests"
else
  exit_success
fi



