#!/bin/sh

###############################################################################
#
# Tests to check that glite-wms* commands behave properly with false input
#
# The tests assume that there is a fake.jdl file in the directory. 
# The file should be syntactically incorrect
# It also assumes there is a working hostname.jdl file in the directory
#
###############################################################################

JDLFILE=hostname.jdl
FAKEJDLFILE=fake.jdl

# --- Special echo ---
function myecho() {

  echo " # glite-wms error test # $@"
}

# --- "Bad" exit ---
function exit_failure() {

  echo ""
  myecho "Started: $START_TIME"
  myecho "Ended  :" `date +%H:%M:%S`

  echo ""
  echo " *** something went wrong *** "
  echo " *** test NOT passed *** "

  exit $1
}

# --- "Good" exit ---
function exit_success() {

  echo ""
  myecho "Started: $START_TIME"
  myecho "Ended  :" `date +%H:%M:%S`

  echo ""
  echo "    === test PASSED === "

  exit 0;
}

myecho "Testing with a false endpoint..."

glite-wms-job-list-match -a -e https://not.a.server.com:7443/glite_wms_wmproxy_server $JDLFILE

if [ $? -eq 0 ] ; then
  myecho "Using a bad service endpoint does not give an error, even if it shoule"
  exit_failure
fi

myecho "Submitting to a bad service endpoint gives an error as it should"


myecho "Testing with a bad jdl..."

glite-wms-job-list-match -a $FAKEJDLFILE

if [ $? -eq 0 ] ; then
  myecho "Using a erroneous jdl does not result in error as it should"
  exit_failure
fi

myecho "Submitting a bad jdl gives an error, as it should"

X509_USER_PROXY=/non/existent/file glite-wms-job-list-match -a  $JDLFILE
if [ $? -eq 0 ] ; then
  myecho "Using a noexistent proxy file does not give an error as it should"
  exit_failure
fi

myecho "Using a nonexistent proxy gives an error, as it should"

TMP_SUBMIT_OUTPUT=/tmp/job_submit_`id -u`_$$

myecho "Submtting a job..."

glite-wms-job-submit -a $JDLFILE > $TMP_SUBMIT_OUTPUT 2>&1

if [ $? -ne 0 ]; then

  cat $TMP_SUBMIT_OUTPUT
  myecho "ERROR: could not submit a job"
  rm -f $TMP_SUBMIT_OUTPUT
  exit_failure 1

else

  cat $TMP_SUBMIT_OUTPUT
  myecho "submit succeeded"
  JOBID=$(cat $TMP_SUBMIT_OUTPUT | grep -v Connecting | grep -v endpoint | grep -o 'https:[[:alnum:].:_/-]*')
  myecho "JOBID: $JOBID"
  rm -f $TMP_SUBMIT_OUTPUT
fi

myecho "Trying to check a job with a false job status"
FAKEID=`echo $JOBID | sed -e "s/\(:[0-9]\{4,5\}\/\).*/\1asdfghjkplzxcvbnmqwert/"`

glite-wms-job-status $FAKEID

if [ $? -eq 0 ] ; then
  myecho "ERROR: giving a false jobid does not result in an error"
  exit_failure 1
fi

myecho "Using a false job id to check job status gives an error, as it should"

myecho "Sleeping a bit..."
sleep 30

myecho "Canceling the job..."

echo y | glite-wms-job-cancel $JOBID

if [ $? -ne 0 ] ; then
  myecho "ERROR: Error canceling the job"
  exit_failure 1
fi

myecho "Testjob cancelled, now we'll sleep a bit..."

sleep 60

myecho "Trying to cancel the already cancelled testjob"

echo y |glite-wms-job-cancel $JOBID

if [ $? -eq 0 ] ; then
  myecho "ERROR: Cancellin an already cancelled job does not give an error"
  exit_failure 1
fi

myecho "Cancelling the already cancelled job gives an error, as it should"

exit_success 
