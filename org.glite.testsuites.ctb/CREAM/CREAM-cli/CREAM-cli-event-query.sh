#!/bin/sh

###############################################################################
#
# Test glite-ce-event-query command:
#
# TEST 1: submit a job and then retrieve all pending events
# TEST 2: submit a job and then retrieve all events from the last one registered
# TEST 3: check if the --conf option works
# TEST 4: save info into a logfile to check if --debug and --logfile options work
#
# Author: Paolo Andreetto <sa3-italia@mi.infn.it>
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

FAILED=0
LASTEVENTREGISTERED=0

prepare $@

my_echo ""

TESTCOMMAND="${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-event-query"

if [ ! -x ${TESTCOMMAND} ] ; then
  exit_failure "Command ${TESTCOMMAND} not exists, test could not be performed!"
fi

my_echo "Purging all existing jobs"

run_command "${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-purge -N -a -e $ENDPOINT"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

sleep 60

####

my_echo "TEST 1: submit a job and then retrieve all pending events:"

run_command "${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-submit -a -r $CREAM $JDLFILE"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

extract_jobid ${COM_OUTPUT}
debug "The job '$JOBID' has been successfully submitted"

run_command "${TESTCOMMAND} -e $ENDPOINT ${LASTEVENTREGISTERED}-10000000"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
  ((FAILED++))
else
  get_last_event_id ${COM_OUTPUT}
  LASTEVENTREGISTERED=${LASTEVENTID}
  debug "Last event ID is ${LASTEVENTID}"
  success
fi

####

my_echo "TEST 2: submit a job and then retrieve all events from the last one registered:"

run_command "${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-submit -a -r $CREAM $JDLFILE"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

extract_jobid ${COM_OUTPUT}
debug "The job '$JOBID' has been successfully submitted"

run_command "${TESTCOMMAND} -e $ENDPOINT ${LASTEVENTREGISTERED}-10000000"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
  ((FAILED++))
else
  get_last_event_id ${COM_OUTPUT}
  LASTEVENTREGISTERED=${LASTEVENTID}
  debug "Last event ID is ${LASTEVENTID}"
  success
fi

####

my_echo "TEST 3: check if the --conf option works:"

mkdir ${MYTMPDIR}/query_log_dir || exit_failure "Cannot create ${MYTMPDIR}/query_log_dir";
printf "[
EVENTQUERY_LOG_DIR=\"${MYTMPDIR}/query_log_dir\";
]
" > ${MYTMPDIR}/query.conf
run_command "${TESTCOMMAND} --debug --conf ${MYTMPDIR}/query.conf -e $ENDPOINT ${LASTEVENTREGISTERED}-10000000"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  RESULT=`ls ${MYTMPDIR}/query_log_dir/* | grep glite-ce-event-query_CREAM | wc -l 2>/dev/null`
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

run_command "${TESTCOMMAND} --debug --logfile ${LOGFILE} -e $ENDPOINT ${LASTEVENTREGISTERED}-10000000"
RESULT=`grep "#HEADER#" ${LOGFILE}`
if [ -z "$RESULT" ]; then
  failure "File ${LOGFILE} has been overwritten"
	((FAILED++)) # continue
else
  RESULT=`grep -E "INFO|ERROR|WARN" ${LOGFILE}`
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
  exit_failure "$FAILED test(s) failed on 4 differents tests"
else
  exit_success
fi

