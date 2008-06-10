#!/bin/sh

###############################################################################
#
# A basic job purge test.
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
my_echo "Submit some jobs to prepare the context of the tests"

i=0
n=9

while [ $i -lt $n ] ; do
  run_command "glite-ce-job-submit -a -o $MYTMPDIR/jobid -r $CREAM $JDLFILE"
  if [ $? -ne 0 ]; then
    exit_failure ${COM_OUTPUT}
  fi
  ((i++))
done

my_echo ""

####

my_echo "TEST 1: purge a finished job"

wait_until_job_finishes

# purge the job
run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-purge -N $JOBID
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
else
	success
fi

PURGJOB=$JOBID

####

my_echo "TEST 2: try to purge a running job"

wait_until_job_runs

# purge the job
run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-purge -N $JOBID
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

RESULT=`echo ${COM_OUTPUT} | grep -P "the job has a status not compatible with the JOB_PURGE command"`

if [ -z "$RESULT" ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  success
fi

####

my_echo "TEST 3: purge again a previous purged job"

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-purge -N $PURGJOB
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

RESULT=`echo ${COM_OUTPUT} | grep -P "job not found"`
if [ -z "$RESULT" ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  success
fi

####


my_echo "TEST 4: purge more finished jobs"

wait_until_job_finishes
JOB1=${JOBID}
wait_until_job_finishes
JOB2=${JOBID}

# purge the job
run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-purge -N ${JOB1} ${JOB2}
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
else
  success
fi

####

my_echo "TEST 5: purge more jobs taking jobID from a file (--input)"

# purge the job
run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-purge -N --input $MYTMPDIR/jobid 
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
else
  success
fi

####

my_echo ""
my_echo "TEST 6: check the requirements of the --all option:"

# -a and -i are not compatible
run_command glite-ce-job-purge -a -i $MYTMPDIR/jobid
if [ $? -ne 1 ]; then
  exit_failure ${COM_OUTPUT}
fi

TMP=`echo ${COM_OUTPUT} | grep "all and --input or --all and specification of JobID(s) as argument are exclusive"`
if [ $? -ne 0 ]; then
  failure " The ouput of the command is: ${COM_OUTPUT}"
  ((FAILED++)) # continue
else
  success
fi

# -a requires -e
run_command glite-ce-job-purge -a
if [ $? -ne 1 ]; then
  exit_failure ${COM_OUTPUT}
fi

TMP=`echo ${COM_OUTPUT} | grep -P "Option --all requires the specification of the endpoint (option --endpoint) to contact"`
if [ $? -ne 0 ]; then
  failure " The ouput of the command is: ${COM_OUTPUT}"
  ((FAILED++)) # continue
else
  success
fi

# extract a jobid
JI=`tail -1 $MYTMPDIR/jobid`

# -a and JOBID are not compatible
run_command glite-ce-job-purge -a $JI
if [ $? -ne 1 ]; then
  exit_failure ${COM_OUTPUT}
fi

TMP=`echo ${COM_OUTPUT} | grep -P "all and --input or --all and specification of JobID(s) as argument are exclusive"`

if [ $? -ne 0 ]; then
  failure " The ouput of the command is: ${COM_OUTPUT}"
  ((FAILED++)) # continue
else
  success
fi
 
####

my_echo "TEST 7: check the --conf option:"

mkdir ${MYTMPDIR}/purge_log_dir || exit_failure "Cannot create ${MYTMPDIR}/purge_log_dir";
printf "[
PURGE_LOG_DIR=\"${MYTMPDIR}/purge_log_dir\";
]
" > ${MYTMPDIR}/purge.conf
run_command glite-ce-job-purge -N --debug --conf ${MYTMPDIR}/purge.conf -i $MYTMPDIR/jobid
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
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

my_echo "TEST 8: purge jobs saving info in a logfile (-d --logfile):"

echo "#HEADER#" > ${LOGFILE} || exit_failure "Cannot open ${LOGFILE}";

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-purge -N -d --logfile ${LOGFILE} -i $MYTMPDIR/jobid
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

my_echo "TEST 9: check the --all option:"

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-purge -N --all -e $ENDPOINT
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
else
	success
fi


# ... terminate

if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 11 differents tests"
else
  exit_success
fi



