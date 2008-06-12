#!/bin/sh

###############################################################################
#
# Test glite-ce-job-list command.
#
# TEST 1: check if the command returns the right number of jobs
# TEST 2: check if the --output option works
# TEST 3: check if the --conf option works
# TEST 4: save info into a logfile to check if --debug and --logfile options work
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

TESTCOMMAND="${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-list"

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

my_echo "TEST 1: check if the command returns the right number of jobs:"

run_command "${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-status -a -e $ENDPOINT > $MYTMPDIR/status_output"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

# Check how many jobs are in the output of the status command
sj=`cat $MYTMPDIR/status_output | grep JobID | wc -l`
debug "Command returns the status of $sj jobs"

run_command "${TESTCOMMAND} $ENDPOINT > $MYTMPDIR/joblist_output"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

# Check how many jobs are in the output of the job-list command
lj=`cat $MYTMPDIR/joblist_output | grep http | wc -l`
debug "Command returns the status of $lj jobs"

if [ $sj -ne $lj ] ; then
  failure "The command returns only $lj jobs (instead of $sj)"
  ((FAILED++)) # continue
else
  success
fi

####

my_echo "TEST 2: check if the --output option works:"

run_command "${TESTCOMMAND} --nomsg --output ${MYTMPDIR}/joblist_file_output $ENDPOINT "
if [ $? -ne 0 ]; then
  exit_failure " COmmand failed: ${COM_OUTPUT}"
fi

# Check how many jobs are in the output file of the job-list command
fj=`cat $MYTMPDIR/joblist_file_output | grep http | wc -l`
debug "Command returns the status of $fj jobs"

if [ $sj -ne $fj ] ; then
  failure "The command returns only $fj jobs (instead of $sj)"
  ((FAILED++)) # continue
else
  success
fi

####

my_echo "TEST 3: check if the --conf option works:"

mkdir ${MYTMPDIR}/joblist_log_dir || exit_failure "Cannot create ${MYTMPDIR}/joblist_log_dir";
printf "[
LIST_LOG_DIR=\"${MYTMPDIR}/joblist_log_dir\";
]
" > ${MYTMPDIR}/joblist.conf

run_command "${TESTCOMMAND} --debug --conf ${MYTMPDIR}/joblist.conf $ENDPOINT"
if [ $? -ne 0 ]; then
  exit_failure "Command failed ${COM_OUTPUT}"
else
  RESULT=`ls ${MYTMPDIR}/joblist_log_dir/* | grep glite-ce-job-list_CREAM | wc -l 2>/dev/null`
  if [ $RESULT == "0" ]; then
    failure "Cannot find debug log file"
    ((FAILED++))
  else
    success
  fi
fi

####

my_echo "TEST 4: save info into a logfile to check if --debug and --logfile options work:"

echo "#HEADER#" > ${LOGFILE} || exit_failure "Cannot open ${LOGFILE}";

run_command "${TESTCOMMAND} --debug --logfile ${LOGFILE} $ENDPOINT"
RESULT=`grep "#HEADER#" ${LOGFILE}`
if [ -z "$RESULT" ]; then
  failure "File ${LOGFILE} has been overwritten"
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

#### FINISHED

if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 4 differents tests"
else
  exit_success
fi


