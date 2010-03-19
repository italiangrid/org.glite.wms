#!/bin/sh

###############################################################################
#
# Test glite-ce-job-status command:
#
# TEST 1: submit a job and then retrieve its status
# TEST 2: submit a job which surely failed. Check if the final status is the ones expected
# TEST 3: check the status of a job setting log level to 1 (option --level 1):
# TEST 4: check the status of a job setting log level to 2 (option --level 2)
# TEST 5: check if the option --input works
# TEST 6: check if the --conf option works
# TEST 7: save info into a logfile to check if --debug and --logfile options work
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

TESTCOMMAND="${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-service-info"

if [ ! -x ${TESTCOMMAND} ] ; then
  exit_failure "Command ${TESTCOMMAND} not exists, test could not be performed!"
fi

####


my_echo "TEST 1: check option --version:"

run_command "${TESTCOMMAND} --version $ENDPOINT > $MYTMPDIR/tmp_output"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

TMP=`grep "CREAM User Interface" $MYTMPDIR/tmp_output`
if [ $? -eq 0 ] ; then
        debug $TMP
        success
else
        failure "The command returns unexpected output: `cat $MYTMPDIR/tmp_output`"
  ((FAILED++)) # continue
fi

####

my_echo "TEST 2: check the service info using various log level:"

run_command "${TESTCOMMAND} $ENDPOINT > $MYTMPDIR/tmp_output"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

TMP=`grep "Interface Version" $MYTMPDIR/tmp_output`
if [ $? -eq 0 ] ; then
	debug $TMP
        success
else
        failure "The command returns unexpected output: `cat $MYTMPDIR/tmp_output`"
  ((FAILED++)) # continue
fi



run_command "${TESTCOMMAND} --nomsg --level 2  $ENDPOINT > $MYTMPDIR/tmp_output"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
fi

TMP=`grep "SUBMISSION_THRESHOLD_MESSAGE" $MYTMPDIR/tmp_output`
if [ $? -eq 0 ] ; then
	success
else
	failure "The command returns unexpected output: `cat $MYTMPDIR/tmp_output`"
  ((FAILED++)) # continue
fi

####

my_echo "TEST 3: check if the --conf option works:"

mkdir ${MYTMPDIR}/status_log_dir || exit_failure "Cannot create ${MYTMPDIR}/status_log_dir";
printf "[
GETSERVICEINFO_LOG_DIR=\"${MYTMPDIR}/status_log_dir\";
]
" > ${MYTMPDIR}/status.conf
run_command "${TESTCOMMAND} --debug --conf ${MYTMPDIR}/status.conf $ENDPOINT"
if [ $? -ne 0 ]; then
  exit_failure "Command failed: ${COM_OUTPUT}"
else
  RESULT=`ls ${MYTMPDIR}/status_log_dir/* | grep glite-ce-service-info| wc -l 2>/dev/null`
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

####


#### FINISHED

if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 4 differents tests"
else
  exit_success
fi

