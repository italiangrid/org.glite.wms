#!/bin/sh

###############################################################################
#
# A job submit - wait - get output test.
# The default is to test the glite workload commands (glite-job-*).
# Alternatively the corresponding EDG or gLite-WMS commands can be tested.
#
# Usage: UI-workload-glite-submit-wait-output.sh -t <wms_type>
#   where wms_type defines what submission system is to be tested.
#   wms_type can be "EDG", "gLite" or "gLite-WMS". The default is gLite.
#
# Features: The test will fail when one of the tested commands fails,
# but not when the job itself finishes with a failure.
# A failure can also be returned in case the job does not finish (or start) 
# within a certain timeout time, and can not be canceled in its current state.
#
# It is assumed that a file hostname.jdl is present in the same directory where the test is.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# --- Special echo ---
function myecho() {

  echo " # $WMSTYPE CLI test # $@"
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


# ------------- main functionality ---------------

# ... get wms type specified with -t, default is gLite

WMSTYPE="gLite"

while getopts ":t:" option; do
  case $option in
    t ) WMSTYPE="$OPTARG";;
    * ) echo "Usage: `basename $0` -t <wms_type>"
        exit 1;;
  esac
done

# ... decide what WMS type is to be tested

case "$WMSTYPE" in

     EDG)
	job_submit='edg-job-submit'
	job_status='edg-job-status'
	job_cancel='edg-job-cancel --noint'
	job_output='edg-job-get-output'
        ;;

     gLite)
	job_submit='glite-job-submit'
	job_status='glite-job-status'
	job_cancel='glite-job-cancel --noint'
	job_output='glite-job-output'
        ;;

     gLite-WMS)
	job_submit='glite-wms-job-submit -a'
	job_status='glite-wms-job-status'
	job_cancel='glite-wms-job-cancel --noint'
	job_output='glite-wms-job-output'
        ;;

     *)
        echo "unknown WMS type: $WMSTYPE";
	exit_failure 1
        ;;
esac

echo ""
echo "    === Test of $WMSTYPE workload management commands ===    "
echo ""

# ... find the jdl file

JDLFILE=$(dirname $0)/hostname.jdl

if [ ! -f "$JDLFILE" ]; then
  myecho "Internal ERROR! could not find example jdl file $JDLFILE"
  exit_failure 1
fi


START_TIME=`date +%H:%M:%S`

# ... submit a job

myecho "current time: $START_TIME"
myecho "submitting a job with ${job_submit}"
echo ""

TMP_SUBMIT_OUTPUT=/tmp/job_submit_`id -u`_$$

${job_submit} $JDLFILE > $TMP_SUBMIT_OUTPUT 2>&1

if [ $? -ne 0 ]; then

  cat $TMP_SUBMIT_OUTPUT
  myecho "ERROR: could not submit a job"
  rm -f $TMP_SUBMIT_OUTPUT
  exit_failure 1

else

  cat $TMP_SUBMIT_OUTPUT
  myecho "submit succeeded"
  JOBID=$(cat $TMP_SUBMIT_OUTPUT | grep -v Connecting | grep -o 'https:[[:alnum:].:_-/]*')
  myecho "JOBID: $JOBID"
  rm -f $TMP_SUBMIT_OUTPUT

fi


myecho "going into status check loop, using  ${job_status}"
echo ""

TMP_STATUS_OUTPUT=/tmp/job_status_`id -u`_$$
NUM_STATUS_RETRIEVALS=40
SLEEP_TIME=15
VERBOSITY_OPTIONS=" --verbosity 2"

# ... loop until job reaches a final state

for ((i=1; i <= ${NUM_STATUS_RETRIEVALS}; i++))
do

  echo ""
  echo "[$i]" `date +%H:%M:%S`

  ${job_status} $VERBOSITY_OPTIONS $JOBID > $TMP_STATUS_OUTPUT 2>&1

  STATUS=$?
  
  cat $TMP_STATUS_OUTPUT

  # ... if job-status failed then exit
  
  if [ $STATUS -ne 0 ]; then

    myecho ' *** ERROR: could not retrieve job status ***'
    myecho "failed command: ${job_status} $JOBID"
    echo ""

    myecho "canceling job $JOBID ..."

    ${job_cancel} $JOBID

    rm -f $TMP_STATUS_OUTPUT

    exit_failure 1

  fi
  
  # ... if time is out then try cancel job and exit

  if [ $i -ge $NUM_STATUS_RETRIEVALS ]; then

    myecho ' *** Timeout reached ***'
    echo ""

    rm -f $TMP_STATUS_OUTPUT

    myecho "canceling job $JOBID ..."

    ${job_cancel} $JOBID

    if [ $? -ne 0 ]; then
      myecho "Could not cancel the job"
      exit_failure 1
    fi

    myecho "Job cancelation succeeded"
    exit_success

  fi
  
  # ... otherwise test its status

  STATUS_LINE=`grep Status: $TMP_STATUS_OUTPUT`
  myecho "Extracted Status line: <${STATUS_LINE}>"
  echo ""
  
  rm -f $TMP_STATUS_OUTPUT

  # ... exit if it is Aborted
  if grep -q "Aborted" <<<"$STATUS_LINE"; then
    myecho "Job was Aborted !"
    exit_success
  fi

  # ... or Canceled
  if grep -q "Cancelled" <<<"$STATUS_LINE"; then
    myecho "The job has been (unexpectedly) cancelled !"
    exit_success
  fi

  # ... or Failed
  if grep -q "Done (Failed)" <<<"$STATUS_LINE"; then
    myecho "The job finished with failure !"
    exit_success
  fi

  # ... go to the next step if it is a success
  if grep -q "Done" <<<"$STATUS_LINE"; then
    myecho "Job finished !"
    break
  fi

  # ... otherwise keep waiting

  echo "sleeping ..."

  sleep $SLEEP_TIME

  VERBOSITY_OPTIONS=""

done

# ... show final job status
echo ""
myecho "current job status: "
${job_status} --verbosity 2 $JOBID

# ... get job output

echo ""
myecho "Getting output of $JOBID using ${job_output} ..."

TMP_OUTPUT_FILE=/tmp/tmp_output_`id -u`_$$

${job_output} "$JOBID" > $TMP_OUTPUT_FILE 2>&1

if [ $? -ne 0 ]; then

  cat $TMP_OUTPUT_FILE
  myecho '*** ERROR: could not retrieve job output ***'
  myecho "failed command: ${job_output} $JOBID"
  rm -f $TMP_OUTPUT_FILE
  exit_failure 1

else

  cat $TMP_OUTPUT_FILE

  # ... extract output directory name from the job-output output using specific part of Job ID ...

  UNIQUE_STRING=`grep -o '[0-9A-Za-z_\-]\{22\}' <<<$JOBID`
  myecho "A unique string extracted from JobID: $UNIQUE_STRING"

  if [ -z "$UNIQUE_STRING" ]; then
    myecho '*** Text parsing ERROR: could not identify directory where the output was stored ***'
    rm -f $TMP_OUTPUT_FILE
    exit_failure 1
  fi

  OUTPUT_DIR=`cat $TMP_OUTPUT_FILE | grep -vF $JOBID | grep -o "[0-9A-Za-z_/.\-]\+$UNIQUE_STRING[0-9A-Za-z_/.\-]*"`
  echo "Output directory extracted from text output: <$OUTPUT_DIR>"

  rm -f $TMP_OUTPUT_FILE

  # ... test if directory name is sensible
  if [ -z "$OUTPUT_DIR" ]; then
    myecho '*** ERROR: could not extract directory name ***'
    exit_failure 1
  fi

  # ... and corresponds to an existing directory
  if [ ! -d "$OUTPUT_DIR" ]; then
    myecho '*** ERROR: directory' $OUTPUT_DIR 'does not exist ***'
    exit_failure 1
  fi

  # ... list it
  myecho "listing contents of $OUTPUT_DIR ..."
  ls -l $OUTPUT_DIR
  echo ""

  # ... test if the files are there and print them out
  
  STD_OUT=$OUTPUT_DIR/std.out
  STD_ERR=$OUTPUT_DIR/std.err

  if [ ! -f "$STD_OUT" ]; then
    myecho '*** ERROR: file' $STD_OUT 'does not exist ***'
    exit_failure 1
  fi

  myecho " --- $STD_OUT ---"
  cat $STD_OUT
  myecho " --- end of $STD_OUT ---"
  echo ""
 
  if [ ! -f "$STD_ERR" ]; then
    myecho '*** ERROR: file' $STD_OUT 'does not exist ***'
    exit_failure 1
  fi

  myecho " --- $STD_ERR ---"
  cat $STD_ERR
  myecho " --- end of $STD_ERR ---"
  echo ""
fi

# ... finish

myecho "finished!"

exit_success

}

# Note: In principle one could do without temporary files as follows:
# JOB_SUBMIT_OUTPUT=`${job_submit} $JDLFILE`;
# if [ $? -ne 0 ]; ...
# ...
# grep -v Connecting <<<"JOB_SUBMIT_OUTPUT"  ...
# But this solution uses an undocumented feature of bash to keep the return code
# of the substitued command after variable assignment.
