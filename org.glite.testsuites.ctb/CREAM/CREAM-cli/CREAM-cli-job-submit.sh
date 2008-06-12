#!/bin/sh

###############################################################################
#
# Test glite-ce-job-submit command:
#
# TEST 1: submit a job without mandatory option: --autm-delegation
# TEST 2: submit a job with automatic delegation (option --autm-delegation)
# TEST 3: submit a job using a previously delegated credentials (options --delegationId)
# TEST 4: check if the option --output works
# TEST 5: check if using again option --output the new joid is append to the existing file
# TEST 6: check if the --conf option works
# TEST 7: save info into a logfile to check if --debug and --logfile options work
#
# Author: Alessio Gianelle <sa3-italia@mi.infn.it>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

FAILED=0

my_echo ""

TESTCOMMAND="${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-submit"

if [ ! -x ${TESTCOMMAND} ] ; then
  exit_failure "Command ${TESTCOMMAND} not exists, test could not be performed!"
fi

####

my_echo "TEST 1: submit a job without mandatory option --autm-delegation:"

run_command "${TESTCOMMAND} --resource $CREAM $JDLFILE"
if [ $? -ne 1 ]; then
  exit_failure "Command unexpected success: ${COM_OUTPUT}"
else
	my_echo "Command returns ${COM_OUTPUT}"
fi

success

####

my_echo "TEST 2: submit a job with automatic delegation (option --autm-delegation):"

run_command "${TESTCOMMAND} --autm-delegation --resource $CREAM $JDLFILE"

if [ $? -ne 0 ]; then
	exit_failure "Command failed: ${COM_OUTPUT}"
fi

extract_jobid ${COM_OUTPUT}
debug "The job $JOBID has been successfully submitted"

success

####

my_echo "TEST 3: submit a job using a previously delegated credentials (options --delegationId):"

# delegate a proxy 
DELEGATION=`new_delegation_id`
run_command "${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-delegate-proxy -e $ENDPOINT $DELEGATION"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

# submit the job
run_command "${TESTCOMMAND} --delegationId $DELEGATION --resource $CREAM $JDLFILE"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

extract_jobid ${COM_OUTPUT}
debug "The job $JOBID has been successfully submitted"

success

####

my_echo "TEST 4: check if the option --output works:"

run_command "${TESTCOMMAND} --autm-delegation --output $MYTMPDIR/jobid --resource $CREAM $JDLFILE"
if [ $? -ne 0 ]; then
  exit_failure "Comamnd failed: ${COM_OUTPUT}"
fi

extract_jobid ${COM_OUTPUT}
debug "Job ${JOBID} has been successfully submitted."

# extract the JobID from the output file
JD=`grep https $MYTMPDIR/jobid`

if [ $JOBID == $JD ] ; then
	success
else
	failure "Test failed: the JobID in the output file: $JD differs from the ones returned by the command: $JOBID"
	((FAILED++)) # continue
fi

####

my_echo "TEST 5: check if using again option --output the new joid is append to the existing file:"

run_command "${TESTCOMMAND} --autm-delegation --output $MYTMPDIR/jobid --resource $CREAM $JDLFILE"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

extract_jobid ${COM_OUTPUT}
debug "Job ${JOBID} has been successfully submitted."

debug "The output file contains these lines:"
debug "`cat $MYTMPDIR/jobid`"

# extract the last JobID saved in the file (last line)
LJD=`tail -1 $MYTMPDIR/jobid`

TMP=`grep $JD $MYTMPDIR/jobid`
if [ $? -eq 1 ] ; then
	failure "Test failed: output file has been overwritten"
	((FAILED++)) # continue
else
	if [ $JOBID == $LJD ] ; then
  	success
	else
  	failure "Test failed: the JobID in the output file: $LJD differs from the one returned by the command: $JOBID"
		((FAILED++)) # continue
	fi
fi

####

my_echo "TEST 6: check if the --conf option works:"

mkdir ${MYTMPDIR}/submit_log_dir || exit_failure "Cannot create ${MYTMPDIR}/submit_log_dir";
printf "[
SUBMIT_LOG_DIR=\"${MYTMPDIR}/submit_log_dir\";
]
" > ${MYTMPDIR}/submit.conf
run_command "${TESTCOMMAND} --delegationId $DELEGATION --debug --conf ${MYTMPDIR}/submit.conf --resource $CREAM $JDLFILE"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  RESULT=`ls ${MYTMPDIR}/submit_log_dir/* | grep glite-ce-job-submit_CREAM | wc -l 2>/dev/null`
  if [ $RESULT == "0" ]; then
    failure "Cannot find debug log file"
    ((FAILED++)) # continue
  else
    success
  fi
fi

####

my_echo "TEST 7: save info into a logfile to check if =--debug= and =--logfile= options work:"

echo "#HEADER#" > ${LOGFILE} || exit_failure "Cannot open ${LOGFILE}";

run_command "${TESTCOMMAND} --delegationId $DELEGATION --debug --logfile ${LOGFILE} --resource $CREAM $JDLFILE"
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
	exit_failure "$FAILED test(s) failed on 7 differents tests"
else
	exit_success
fi
