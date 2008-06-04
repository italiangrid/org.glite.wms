#!/bin/sh

###############################################################################
#
# Common functions for the glite-wms CLI test suite
#
# The package is intended primarily to test the CREAM-CLI installed
# on a UI and not the CREAM service itself or computing infrastructure.
# In particular, this implies that a test should fail when one of the tested
# commands fails, but not when the job itself finishes with a failure or gets
# aborted from the queue. 
#
# The test suite requires example.jdl, test.conf and cream.conf to be present
# in the same directory with the scripts
#
# Author: Alessio Gianelle <sa3-italia@mi.infn.it>
# Version: $Id$
#
###############################################################################

# Special echo
function my_echo()
{
  echo " --> $@"
}

# Print debug messages 
function debug()
{
	if [ $DEBUG == 1 ]; then
		my_echo "DEBUG: $@"
	fi
	return 0
}

# Print failure message, reason in $@
function failure()
{
	my_echo ""
	my_echo " *** TEST NOT PASSED *** "
  my_echo " *** Failure reason: $@ *** "
  my_echo ""

	return 0
}

# ... "bad" exit, the failure reason is stored in the arguments: $@
function exit_failure()
{
  my_echo ""
  cleanup

  my_echo "Started: $START_TIME"
  my_echo "Ended  :" $(date +%H:%M:%S)

	failure $@
	
  exit 1
}

# Print success message, reason in $@
function success()
{
	my_echo ""
  my_echo " === TEST PASSED === "
	my_echo ""

  return 0
}

# ... "good" exit
function exit_success()
{
  my_echo ""
  cleanup

  my_echo "Started: $START_TIME"
  my_echo "Ended  :" $(date +%H:%M:%S)

	success

  exit 0;
}

# ... exit on timeoout: try cancel job and exit
function exit_timeout()
{
  my_echo ' *** Timeout reached ***'
  run_command glite-ce-job-cancel --noint $JOBID
  run_command glite-ce-job-purger --noint $JOBID
  exit_success
}

# ... exit on Ctrl^C
function exit_interrupt()
{
  my_echo ' *** Interrupted by user ***'
  trap - SIGINT
  cleanup
  exit 1
}

# TO CHECK
# ... cleanup temporary files
function cleanup()
{
  my_echo "cleaning up $MYTMPDIR ..."

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

# Run command given as arguments ($@) and put the output in $COM_OUTPUT
# Return 0 if success, 1 otherwise.
function run_command()
{
  my_echo ""
  my_echo "[" $(date +%H:%M:%S) "] run: " $@

  COM_OUTPUT=$(eval $@ 2>&1)

  if [ $? -ne 0 ]; then
		my_echo "Command FAILED"
    debug "Command output was: ${COM_OUTPUT}"
    return 1
  fi

	debug "Command run succesfully. Output is: ${COM_OUTPUT}"
  return 0 
}

# TO CHECK
# ... print help message and exit
function usage()
{
  echo "Usage: "
  echo ""
  echo $(basename $0) "[-d]"
  echo ""
  echo " -d   use glite-ce-delegate-proxy (default behaviour is to use automatic delegation)"
  echo ""
  exit 0
}

# TO CHECK
# ... prepare everything
function prepare()
{

	source test.conf

  if [ "$1" == "--help" ]; then
    usage
    exit 0
  fi
  
  my_echo "++++++++++++++++++++++++++++++++++++++++++++"
  my_echo "+ Test of CREAM-CE  command line interface +"
  my_echo "++++++++++++++++++++++++++++++++++++++++++++"

  START_TIME=$(date +%H:%M:%S)
  my_echo "current time: $START_TIME"

  [[ -f "$JDLFILE"       ]] || exit_failure "Internal ERROR! could not find example jdl file $JDLFILE"
  [[ -f "${CONFIG_FILE}" ]] || exit_failure "Internal ERROR! could not find example config file ${CONFIG_FILE}"

  # ... create temporary directory
  MYTMPDIR=/tmp/cream-cli-test-$(id -un)-$$
  mkdir $MYTMPDIR || exit_failure

  # ... define common directory and file names
  JOB_OUTPUT_DIR=$MYTMPDIR/jobOutput
  LOGFILE=$MYTMPDIR/tmp.log
  CEIDFILE=$MYTMPDIR/ce.id
  TESTFILE=$MYTMPDIR/test.file
  TMPJDLFILE=$MYTMPDIR/test.jdl
  OUTPUTFILE=$MYTMPDIR/output.log
  TMPJOBIDFILE=$MYTMPDIR/job.id

  # ... define delegation parameters
  DELEGATION_OPTIONS="-a"
  if [ "$1" == "-d" ]; then
    define_delegation
  fi

  # ... set a trap for Ctrl^C
  trap exit_interrupt SIGINT
}

# NO USED
# ... delegate proxy and (re-)define DELEGATION_OPTIONS
function define_delegation()
{
  DELEGATION_OPTIONS="-D DelegateId_$$"
  my_echo "delegating proxy ..."
	ENDPOINT=`echo $CREAM | sed -e "s/8443.*/8443/"`
  run_command glite-ce-delegate-proxy -e $ENDPOINT DelegateId_$$
  if [ -n "$SLEEP_AFTER_DELEGATING" ]; then
     my_echo "sleeping $SLEEP_AFTER_DELEGATING seconds ..."
     sleep $SLEEP_AFTER_DELEGATING
  fi
}


# Extract jobid from text lines given by $@ and set JOBID
function extract_jobid()
{
  JOBID=$(echo $@ | grep -m 1 https | sed -e "s/.*https/https/" )
  debug "JobID is: $JOBID"
  if [ -z "$JOBID" ]; then
    exit_failure "ERROR: could not determine Job Id!"
  fi
	return 0
}

# Extract the job STATUS quering the CE with the given JobID: $1 and set JOBSTATUS
function extract_status()
{
  JOBSTATUS=$(glite-ce-job-status -n -L 0 "$1" | grep "Status        = \[.*\]" | sed -e "s/.*\[//" | sed -e "s/\].*//")
  debug "Job status is: $JOBSTATUS"
  if [ -z "$JOBSTATUS" ]; then
    exit_failure "ERROR: could not determine Job STATUS!"
  fi
	return 0
}



# Extract job status of the given JobID ($1), return true if job is finished, exit program if job is Aborted/Cancelled
function is_finished()
{
	extract_status "$1"

  my_echo "Job status is: <${JOBSTATUS}>"

  # ... exit if it is Aborted
  if [[ "$JOBSTATUS" == ABORTED ]]; then
    my_echo "Job was Aborted !"
    exit_success
  fi

  # ... or Cancelled
  if [[ "$JOBSTATUS" == CANCELLED ]]; then
    my_echo "The job has been (unexpectedly) cancelled !"
    exit_success
  fi

  # ... or Failed
  if [[ "$JOBSTATUS" == DONE-FAILED ]]; then
    my_echo "The job finished with failure !"
    exit_success
  fi

  # ... go to the next step if it is a success
  if [[ "$JOBSTATUS" == DONE-OK ]]; then
    my_echo "Job finished !"
    return 0
  fi

  return 1
}

# ... wait until job $JOBID is done or time is out
function wait_until_job_finishes()
{

  i=1

  while ! is_finished "$1"
  do

    if [ $i -ge $NUM_STATUS_RETRIEVALS ]; then
      exit_timeout $JOBID
    fi
  
    my_echo "sleeping for $SLEEP_TIME seconds (run $i/$NUM_STATUS_RETRIEVALS) ..."
    sleep $SLEEP_TIME

    ((i++))

  done

}
