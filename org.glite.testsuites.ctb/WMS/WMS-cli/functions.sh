#!/bin/sh

###############################################################################
#
# Common functions for the glite-wms CLI test suite
#
# The package is intended primarily to test the WMS client software installed
# on a UI and not the WMS service itself or computing infrastructure.
# In particular, this implies that a test should fail when one of the tested
# commands fails, but not when the job itself finishes with a failure or gets
# aborted from the queue. A success will also be returned in case a job did not
# finish, or even start, within certain period of time, providing that it could
# be successfully cancelled.
#
# The test suite requires hostname.jdl, true.jdl and wms.conf to be present
# in the same directory with the scripts
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... special echo
function myecho()
{
  echo " ===> $@"
}

# ... "bad" exit
function exit_failure()
{
  echo ""
  cleanup

  myecho "Started: $START_TIME"
  myecho "Ended  :" $(date +%H:%M:%S)
  echo ""
  echo " *** something went wrong *** "
  echo " *** failure reason: $1 *** "
  echo " *** test NOT passed *** "

  exit 1
}

# ... "good" exit
function exit_success()
{
  echo ""
  cleanup

  myecho "Started: $START_TIME"
  myecho "Ended  :" $(date +%H:%M:%S)

  echo ""
  echo "    === test PASSED === "

  exit 0;
}

# ... exit on timeoout: try cancel job and exit
function exit_timeout()
{
  myecho ' *** Timeout reached ***'
  run_command glite-wms-job-cancel --noint $JOBID
  run_command glite-wms-job-status --verbosity 2 $JOBID
  exit_success
}

# ... exit on Ctrl^C
function exit_interrupt()
{
  myecho ' *** Interrupted by user ***'
  trap - SIGINT
  cleanup
  exit 1
}

# ... cleanup temporary files
function cleanup()
{
  myecho "cleaning up $MYTMPDIR ..."

  [[ -f $OUTPUTFILE   ]] && rm -f $OUTPUTFILE
  [[ -f $LOGFILE      ]] && rm -f $LOGFILE
  [[ -f $CEIDFILE     ]] && rm -f $CEIDFILE
  [[ -f $TMPJOBIDFILE ]] && rm -f $TMPJOBIDFILE
  [[ -f $TMPJDLFILE   ]] && rm -f $TMPJDLFILE
  [[ -f $TESTFILE     ]] && rm -f $TESTFILE

  for OUTFILE in $(ls ${JOB_OUTPUT_DIR}/out.txt-* 2>/dev/null)
  do
    rm -f $OUTFILE
  done

  [[ -f ${JOB_OUTPUT_DIR}/std.out ]] && rm -f ${JOB_OUTPUT_DIR}/std.out
  [[ -f ${JOB_OUTPUT_DIR}/std.err ]] && rm -f ${JOB_OUTPUT_DIR}/std.err
  [[ -f ${JOB_OUTPUT_DIR}/out.txt ]] && rm -f ${JOB_OUTPUT_DIR}/out.txt
  [[ -d ${JOB_OUTPUT_DIR}         ]] && rmdir ${JOB_OUTPUT_DIR}

  [[ -d "$MYTMPDIR" ]] && rmdir $MYTMPDIR
}

# ... run command succussfuly or exit with cleanup
function run_command()
{
  echo ""
  echo "[" $(date +%H:%M:%S) "] $" $@

  eval $@

  if [ $? -ne 0 ]; then
    exit_failure "$1 failed"
  fi

  return 0
}

# ... run command succussfuly or exit with cleanup, keep command output in $OUTPUT
function run_command_keep_output()
{
  echo ""
  echo "[" $(date +%H:%M:%S) "] $" $@

  OUTPUT=$(eval $@ 2>&1)

  if [ $? -ne 0 ]; then
    echo "${OUTPUT}"
    exit_failure "$1 failed"
  fi

  echo "${OUTPUT}"
  return 0
}

# ... print help message and exit
function usage()
{
  echo "Usage: "
  echo ""
  echo $(basename $0) "[-d]"
  echo ""
  echo " -d   use glite-wms-delegate-proxy (default behaviour is to use automatic delegation)"
  echo ""
  exit 0
}

# ... prepare everything
function prepare()
{
  if [ "$1" == "--help" ]; then
    usage
    exit 0
  fi
  
  myecho "++++++++++++++++++++++++++++++++++++++++++++"
  myecho "+ Test of gLite-WMS command line interface +"
  myecho "++++++++++++++++++++++++++++++++++++++++++++"

  START_TIME=$(date +%H:%M:%S)
  myecho "current time: $START_TIME"

  # ... locate jdl files
  JDLFILE=$(dirname $0)/hostname.jdl
  TRUEJDL=$(dirname $0)/true.jdl
  CONFIG_FILE=$(dirname $0)/wms.conf

  [[ -f "$JDLFILE"       ]] || exit_failure "Internal ERROR! could not find example jdl file $JDLFILE"
  [[ -f "$TRUEJDL"       ]] || exit_failure "Internal ERROR! could not find example jdl file $TRUEJDL"
  [[ -f "${CONFIG_FILE}" ]] || exit_failure "Internal ERROR! could not find example config file ${CONFIG_FILE}"

  # ... create temporary directory
  MYTMPDIR=/tmp/wms-cli-test-$(id -un)-$$
  mkdir $MYTMPDIR || exit_failure

  # ... define common directory and file names
  JOB_OUTPUT_DIR=$MYTMPDIR/jobOutput
  LOGFILE=$MYTMPDIR/tmp.log
  CEIDFILE=$MYTMPDIR/ce.id
  TESTFILE=$MYTMPDIR/test.file
  TMPJDLFILE=$MYTMPDIR/test.jdl
  OUTPUTFILE=$MYTMPDIR/output.log
  TMPJOBIDFILE=$MYTMPDIR/job.id

  # ... define "waiting loop" parameters
  NUM_STATUS_RETRIEVALS=40
  SLEEP_TIME=15

  # ... artificial delay before submitting a job using previousely delegated proxy
  SLEEP_AFTER_DELEGATING=5

  # ... define delegation parameters
  DELEGATION_OPTIONS="-a"
  if [ "$1" == "-d" ]; then
    define_delegation
  fi

  # ... set a trap for Ctrl^C
  trap exit_interrupt SIGINT
}

# ... delegate proxy and (re-)define DELEGATION_OPTIONS
function define_delegation()
{
  DELEGATION_OPTIONS="-d $$"
  myecho "delegating proxy ..."
  run_command glite-wms-job-delegate-proxy $DELEGATION_OPTIONS
  if [ -n "$SLEEP_AFTER_DELEGATING" ]; then
     myecho "sleeping $SLEEP_AFTER_DELEGATING seconds ..."
     sleep $SLEEP_AFTER_DELEGATING
  fi
}

# ... take jobid from file given by $1 and cleanup the file
function extract_jobid()
{
  JOBID=$(grep -m 1 https < "$1")
  myecho "JOBID: $JOBID"
  if [ -z "$JOBID" ]; then
    exit_failure "ERROR: could not determine Job Id!"
  fi
}

# ... Extract job status from text given in $1, return true if job is finished, exit program if job is Aborted/Cancelled
function is_finished()
{
  STATUS_LINE=$(grep -i -m 1 Status: <<<"$1")
  myecho "Status line: <${STATUS_LINE}>"

  # ... exit if it is Aborted
  if [[ "$STATUS_LINE" == *Aborted* ]]; then
    myecho "Job was Aborted !"
    exit_success
  fi

  # ... or Canceled
  if [[ "$STATUS_LINE" == *Cancelled* ]]; then
    myecho "The job has been (unexpectedly) cancelled !"
    exit_success
  fi

  # ... or Failed
  if [[ "$STATUS_LINE" == *Done*Failed* ]]; then
    myecho "The job finished with failure !"
    exit_success
  fi

  # ... go to the next step if it is a success
  if [[ "$STATUS_LINE" == *Done* ]]; then
    myecho "Job finished !"
    return 0
  fi

  return 1
}

# ... wait until job $JOBID is done or time is out
function wait_until_job_finishes()
{
  run_command_keep_output glite-wms-job-status --verbosity 2 $JOBID

  i=1

  while ! is_finished "$OUTPUT"
  do

    if [ $i -ge $NUM_STATUS_RETRIEVALS ]; then
      exit_timeout $JOBID
    fi
  
    myecho "sleeping ($i) ..."
    sleep $SLEEP_TIME

    run_command_keep_output glite-wms-job-status $JOBID

    ((i++))

  done

  run_command glite-wms-job-status --verbosity 2 $JOBID
}
