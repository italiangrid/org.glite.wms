#!/bin/sh

###############################################################################
#
# A basic job submit test.
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

my_echo "TEST 1: submit a job without mandatory options (-a -r):"

run_command glite-ce-job-submit -r $CREAM $JDLFILE
if [ $? -ne 1 ]; then
  exit_failure ${COM_OUTPUT}
else
	my_echo "Command returns ${COM_OUTPUT}"
fi

success

####

my_echo "TEST 2: submit a job with mandatory options (-a -r):"

run_command glite-ce-job-submit -a -r $CREAM $JDLFILE

if [ $? -ne 0 ]; then
	exit_failure ${COM_OUTPUT}
fi

extract_jobid ${COM_OUTPUT}
debug "The job $JOBID has been successfully submitted"

success

####

my_echo "TEST 3: submit a job after the proxy delegation (-D):"

# delegate a proxy 
ENDPOINT=`echo $CREAM | sed -e "s/8443.*/8443/"`
run_command glite-ce-delegate-proxy -e $ENDPOINT DelegateId_$$
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

# submit the job
run_command glite-ce-job-submit -D DelegateId_$$ -r $CREAM $JDLFILE
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

extract_jobid ${COM_OUTPUT}
debug "The job $JOBID has been successfully submitted"

success

####

my_echo "TEST 4: submit a job storing the JobID in a file (--output):"

run_command glite-ce-job-submit --autm-delegation --output $TMPJOBIDFILE -r $CREAM $JDLFILE
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

extract_jobid ${COM_OUTPUT}
debug "Job ${JOBID} has been successfully submitted."

# extract the JobID from the output file
JD=`grep https $TMPJOBIDFILE`

if [ $JOBID == $JD ] ; then
	success
else
	failure "Test failed: the JobID in the output file $JD differs from the one returned by the command $JOBID"
	((FAILED++)) # continue
fi

####

my_echo "TEST 5: submit a job storing the JobID in the already used file (--output):"

run_command glite-ce-job-submit --autm-delegation --output $TMPJOBIDFILE -r $CREAM $JDLFILE
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

extract_jobid ${COM_OUTPUT}
debug "Job ${JOBID} has been successfully submitted."

debug "The output file contains these lines:"
debug "`cat $TMPJOBIDFILE`"

# extract the last JobID saved in the file (last line)
JD=`tail -1 $TMPJOBIDFILE`

if [ $JOBID == $JD ] ; then
  success
else
  failure "Test failed: the JobID in the output file $JD differs from the one returned by the command $JOBID"
	((FAILED++)) # continue
fi

####

my_echo "TEST 6: submit a job setting a logfile (-d --logfile):"

run_command glite-ce-job-submit --delegationId DelegateId_$$ -d --logfile $LOGFILE -r $CREAM $JDLFILE
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

extract_jobid ${COM_OUTPUT}
debug "Job ${JOBID} has been successfully submitted"

if [ -e ${LOGFILE} ] ; then
	my_echo ""
	my_echo "Logfile contains:"
	my_echo ""
	my_echo "`cat $LOGFILE`"
	success
else
	failure "Logfile doesn't exist!"
	((FAILED++)) # continue
fi	

####

my_echo "TEST 7: submit a job setting a configuration file (--conf):"

mkdir ${MYTMPDIR}/submit_log_dir || exit_failure "Cannot create ${MYTMPDIR}/submit_log_dir";
printf "[
SUBMIT_LOG_DIR=\"${MYTMPDIR}/submit_log_dir\";
]
" > ${MYTMPDIR}/submit.conf
run_command glite-ce-job-submit --delegationId DelegateId_$$ -d --conf ${MYTMPDIR}/submit.conf -r $CREAM $JDLFILE
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
else
  RESULT=`ls ${MYTMPDIR}/submit_log_dir/* | grep glite-ce-job-submit_CREAM | wc -l 2>/dev/null`
  if [ $RESULT == "0" ]; then
    failure "Cannot find debug log file"
    ((FAILED++)) # continue
  else
    success
  fi
fi


# ... terminate

if [ $FAILED -gt 0 ] ; then
	exit_failure "$FAILED test(s) failed on 7 differents tests"
else
	exit_success
fi
