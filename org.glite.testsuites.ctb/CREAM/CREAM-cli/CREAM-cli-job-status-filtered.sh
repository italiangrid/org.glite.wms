#!/bin/sh

###############################################################################
#
# Test the "filtered" options of the glite-ce-job-status commad.
#
# TEST 1: check if the --from option works:
# TEST 2: check the --from and --to options together
# TEST 3: check if the --status option works (using REALLY-RUNNING)
# TEST 4: check if the --status option works (using DONE-OK)
# TEST 5: check if the --all option works
# TEST 6: check the requirements of the --all option (3 cases)
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

TESTCOMMAND="${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-status"

if [ ! -x ${TESTCOMMAND} ] ; then
  exit_failure "Command ${TESTCOMMAND} not exists, test could not be performed!"
fi

####

my_echo "Submit some jobs to prepare the context of the tests"

i=0
n=9

while [ $i -lt $n ] ; do
	run_command "${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-submit -a -o $MYTMPDIR/jobid -r $CREAM $JDLFILE"
	if [ $? -ne 0 ]; then
		exit_failure "Command failed: ${COM_OUTPUT}"
	fi
	((i++))
done

CHECK_TIME=$(date +%H:%M:%S)

i=0
m=7

while [ $i -lt $m ] ; do
  run_command "${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-submit -a -o $MYTMPDIR/jobid -r $CREAM $JDLFILE"
  if [ $? -ne 0 ]; then
    exit_failure "Command failed: ${COM_OUTPUT}"
  fi
  ((i++))
done

####

my_echo ""
my_echo "TEST 1: check if the --from option works:"

run_command "${TESTCOMMAND} --all --endpoint $ENDPOINT --from \"`date +%Y-%m-%d` $START_TIME\" > $MYTMPDIR/tmp_output"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

# Check how many jobs the command returns
rj=`cat $MYTMPDIR/tmp_output | grep JobID | wc -l`
debug "Command returns the status of $rj jobs"

if [ $rj -ne $(($n+$m)) ] ; then
  failure "The command returns tha status of only $rj jobs (instead of $(($n+$m)) )"
  ((FAILED++)) # continue
else
  success
fi

####

my_echo "TEST 2: check the --from and --to options together:"

run_command "${TESTCOMMAND} --all --endpoint $ENDPOINT --from \"`date +%Y-%m-%d` $START_TIME\" --to \"`date +%Y-%m-%d` $CHECK_TIME\"  > $MYTMPDIR/tmp_output"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

# Check how many jobs the command returns
rj=`cat $MYTMPDIR/tmp_output | grep JobID | wc -l`
debug "Command returns the status of $rj jobs"

if [ $rj -ne $n ] ; then
  failure "The command returns the status of only $rj jobs (instead of $n)"
  ((FAILED++)) # continue
else
  success
fi

####

my_echo "TEST 3: check if the --status option works (using REALLY-RUNNING):"

run_command "${TESTCOMMAND} --all --endpoint $ENDPOINT --from \"`date +%Y-%m-%d` $START_TIME\" --status \"REALLY-RUNNING\"  > $MYTMPDIR/tmp_output"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

# Check how many jobs are in the output of the command 
rj=`cat $MYTMPDIR/tmp_output | grep JobID | wc -l`
debug "Command returns the status of $rj jobs"

# Check how many jobs are in the REALLY-RUNNING status
rr=`cat $MYTMPDIR/tmp_output | grep "REALLY-RUNNING" | wc -l`
debug "Command returns $rr jobs in REALLY-RUNNING status"

if [ $rj -ne $rr ] ; then
  failure "In the output of the command there are some jobs not in the right status"
  ((FAILED++)) # continue
else
  success
fi

####

my_echo "TEST 4: check if the --status option works (using DONE-OK):"

# Do a generic status of all the jobs
run_command "${TESTCOMMAND} --input $MYTMPDIR/jobid  > $MYTMPDIR/tmp_output"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

# Query only the status of the "DONE-OK" jobs
run_command "${TESTCOMMAND} --all --endpoint $ENDPOINT --from \"`date +%Y-%m-%d` $START_TIME\" --status \"DONE-OK\"  > $MYTMPDIR/done_output"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

# Check how many jobs are in DONE status using the output of the general command
ds=`cat $MYTMPDIR/tmp_output | grep DONE-OK | wc -l`
debug "There are $ds jobs in DONE-OK status"

# Check how many jobs are in DONE status using the output of the command with the "--status" option
rj=`cat $MYTMPDIR/done_output | grep JobID | wc -l`
debug "Command returns the status of $rj jobs"

# the number of jobs returned by the second command must be >= 
# of the "DONE-OK" jobs given by the first command
if [ $rj -lt $ds ] ; then
  failure "The command returns the status of only $rj jobs (instead of at least $ds)"
  ((FAILED++)) # continue
else
  success
fi

####

my_echo "TEST 5: check if the --all option works:"

run_command "${TESTCOMMAND} --nomsg --all --endpoint $ENDPOINT"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  success
fi

####

my_echo "TEST 6: check the requirements of the --all option (3 cases):"

# -a and -i are not compatible
run_command "${TESTCOMMAND} --nomsg  --all --input $MYTMPDIR/jobid"
if [ $? -ne 1 ]; then
  exit_failure "Command unexpected success: ${COM_OUTPUT}"
fi

# check the error message
TMP=`echo ${COM_OUTPUT} | grep "Cannot specify Job IDs as command line argument with --all option"`
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
TMP=`echo ${COM_OUTPUT} | grep "all option requires --endpoint."`

if [ $? -ne 0 ]; then
  failure " The ouput of the command is: ${COM_OUTPUT}"
  ((FAILED++)) # continue
else
  success
fi

# extract a jobid
JI=`tail -1 $MYTMPDIR/jobid`

# -a and JOBID are not compatible
run_command "${TESTCOMMAND} --nomsg  --all $JI"
if [ $? -ne 1 ]; then
  exit_failure "Command unexpected success: ${COM_OUTPUT}"
fi

# check the error message
TMP=`echo ${COM_OUTPUT} | grep "Cannot specify Job IDs as command line argument with --all option"`
if [ $? -ne 0 ]; then
  failure " The ouput of the command is: ${COM_OUTPUT}"
  ((FAILED++)) # continue
else
  success
fi

####

#### FINISHED

if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 8 differents tests"
else
  exit_success
fi




