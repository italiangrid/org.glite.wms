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

FAILED=0

prepare $@

my_echo ""

my_echo "TEST 1: submit a job and then retrieve its status:"

run_command glite-ce-job-submit -a -r $CREAM $JDLFILE
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

extract_jobid ${COM_OUTPUT}
debug "The job '$JOBID' has been successfully submitted"

run_command glite-ce-job-status ${JOBID}
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
else
	success
fi

#wait_until_job_finishes ${JOBID}


####

my_echo ""

my_echo "TEST 2: submit a job which surely failed. Check if at the end the job failed:"

## use a wrong queue
FAILCREAM=`echo $CREAM |  sed -e "s/$QUEUE/queuenotexist/"`

run_command glite-ce-job-submit -a -r $FAILCREAM $JDLFILE
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

extract_jobid ${COM_OUTPUT}
debug "The job '$JOBID' has been successfully submitted"

wait_until_job_finishes ${JOBID}

if [ $? -eq 1 ] ; then
	success
else
	failure "Job finished with wrong status $JOBSTATUS"
  ((FAILED++)) # continue
fi

####

my_echo ""

my_echo "TEST 3: check the status of a job setting log level = 1 (-L 1):"

run_command glite-ce-job-submit -a -r $CREAM $JDLFILE
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

extract_jobid ${COM_OUTPUT}
debug "The job '$JOBID' has been successfully submitted"

run_command "glite-ce-job-status -n -L 1 ${JOBID} > $MYTMPDIR/tmp_output"
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

TMP=`grep "Grid JobID" $MYTMPDIR/tmp_output`

if [ $? -eq 0 ] ; then
	success
else
	failure "The command returns unexpected output: `cat $MYTMPDIR/tmp_output`"
  ((FAILED++)) # continue
fi

####

my_echo ""

my_echo "TEST 4: check the status of a job setting log level = 2 (-L 2):"

run_command glite-ce-job-submit -a -r $CREAM $JDLFILE
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

extract_jobid ${COM_OUTPUT}
debug "The job '$JOBID' has been successfully submitted"

run_command "glite-ce-job-status -n -L 2 ${JOBID} > $MYTMPDIR/tmp_output"
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

TMP=`grep "Local User" $MYTMPDIR/tmp_output`

if [ $? -eq 0 ] ; then
  success
else
  failure "The command returns unexpected output: `cat $MYTMPDIR/tmp_output`"
  ((FAILED++)) # continue
fi

####

my_echo ""

# Define how many jobs submit
n=7

my_echo "TEST 5: check the status of a set of jobs ($n) saved in a file (-i):"

# Submit n jobs

i=0

while [ $i -lt $n ] ; do
	run_command "glite-ce-job-submit -a -o $MYTMPDIR/jobid -r $CREAM $JDLFILE"
		if [ $? -ne 0 ]; then
  		exit_failure ${COM_OUTPUT}
		fi
	((i++))
done

# Check if n jobs has been submitted

nj=`cat $MYTMPDIR/jobid | grep https | wc -l`
if [ $nj -ne $n ] ; then 
	exit_failure "I found only $nj jobs into the file; something goes wrong!"
fi

# Query the CE

run_command "glite-ce-job-status -n -i $MYTMPDIR/jobid  > $MYTMPDIR/tmp_output2"
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

# Check how many jobs the command returns

rj=`cat $MYTMPDIR/tmp_output2 | grep JobID | wc -l`

if [ $rj -ne $n ] ; then
  failure "The command returns tha status of only $rj jobs (instead of $n)"
	((FAILED++)) # continue
else
	success
fi

#### FINISHED

if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 5 differents tests"
else
  exit_success
fi

