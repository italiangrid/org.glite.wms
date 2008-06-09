#!/bin/sh

###############################################################################
#
# Common functions for the CREAM-CLI test suite
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
# Author: SA3-IT <sa3-italia@mi.infn.it>
# Version: $Id$
#
###############################################################################

# Special echo
# Parameter: the message to print ($@)
function my_echo()
{
  echo " --> $@"
}

# Print debug messages 
# Parameter: the debug message ($@)
function debug()
{
	if [ $DEBUG == 1 ]; then
		my_echo "DEBUG: $@"
	fi
	return 0
}

# Print failure message
# Parameter: the failure reason ($@)
function failure()
{
	my_echo ""
	my_echo " *** TEST NOT PASSED *** "
  my_echo " *** Failure reason: $@ *** "
  my_echo ""

	return 0
}

# Exit the program with failure
# Parameter: the failure reason ($@)
function exit_failure()
{
  my_echo ""
  cleanup

  my_echo "Started: $START_TIME"
  my_echo "Ended  :" $(date +%H:%M:%S)

	failure $@
	
  exit 1
}

# Print success message
function success()
{
	my_echo ""
  my_echo " === TEST PASSED === "
	my_echo ""

  return 0
}

# Exit the program with success
function exit_success()
{
  my_echo ""
  cleanup

  my_echo "Started: $START_TIME"
  my_echo "Ended  :" $(date +%H:%M:%S)

	success

  exit 0;
}

# Exit on timeout: try to cancel and purge the job before exiting
# Parameter: The jobid ($1)
function exit_timeout()
{
  run_command glite-ce-job-cancel -n $1
  run_command glite-ce-job-purger -n $1
  exit_failure " *** Timeout reached *** "
}

# Exit on Ctrl^C
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

  [[ -d "$MYTMPDIR" ]] && rm -rf $MYTMPDIR

	return 0
}

# Execute the given command 
# Parameter: the command ($@) 
# Set: COM_OUTPUT with the command output
# Return: 0 if success, 1 otherwise
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

	debug "Command runs succesfully. Output is: ${COM_OUTPUT}"
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
	# Sources global variables (test.conf is REQUIRED)
	[[ -f test.conf ]] || exit_failure "Internal ERROR! Could not find the required 'test.conf' file!"
	source test.conf

#  if [ "$1" == "--help" ]; then
#    usage
#    exit 0
#  fi
  
  my_echo "++++++++++++++++++++++++++++++++++++++++++++"
  my_echo "+ Test of CREAM-CE  command line interface +"
  my_echo "++++++++++++++++++++++++++++++++++++++++++++"

  START_TIME=$(date +%H:%M:%S)
  my_echo "Test start at: $START_TIME"

	# The JDLFILE and the CONFIG_FILE are required!
  [[ -f "$JDLFILE"       ]] || exit_failure "Internal ERROR! Could not find example jdl file $JDLFILE"
  [[ -f "${CONFIG_FILE}" ]] || exit_failure "Internal ERROR! Could not find example config file ${CONFIG_FILE}"

  # ... create temporary directory
  MYTMPDIR=/tmp/cream-cli-test-$(id -un)-$$
  mkdir $MYTMPDIR || exit_failure "Internal ERROR! Could not create temporary directory $MYTMPDIR"

	# CHECK if all of them are needed...
  # ... define common directory and file names
  JOB_OUTPUT_DIR=$MYTMPDIR/jobOutput
  LOGFILE=$MYTMPDIR/tmp.log
  CEIDFILE=$MYTMPDIR/ce.id
  TESTFILE=$MYTMPDIR/test.file
  TMPJDLFILE=$MYTMPDIR/test.jdl
  OUTPUTFILE=$MYTMPDIR/output.log
  TMPJOBIDFILE=$MYTMPDIR/job.id

	# Set the auxiliar variables "ENDPOINT" and "QUEUE"
  ENDPOINT=`echo $CREAM | awk -F'/' '{print $1}'`
	QUEUE=`echo $CREAM | awk -F'-' '{print $3}'`


  # Set a trap for Ctrl^C
  trap exit_interrupt SIGINT
}

# Extract the jobid
# Parameter: the output of a submit command ($@)
# Set: JOBID with the jobid of the job
# Return: 0 or exit with failure
function extract_jobid()
{
  JOBID=$(echo $@ | grep -m 1 https | sed -e "s/.*https/https/" )
  debug "JobID is: $JOBID"
  if [ -z "$JOBID" ]; then
    exit_failure "ERROR: could not determine Job Id!"
  fi
	return 0
}

# Extract the job status quering the CE
# Parameter: the jobid of the job ($1)
# Set: JOBSTATUS with the status of the job
# Return: 0 or exit with failure
function extract_status()
{
  JOBSTATUS=$(glite-ce-job-status -n -L 0 "$1" | awk -F'[\\\[\\\]]' '/Status/ {print $2}' - 2>/dev/null)
  debug "Job status is: $JOBSTATUS"
  if [ -z "$JOBSTATUS" ]; then
    exit_failure "ERROR: could not determine Job STATUS!"
  fi
	return 0
}

# Check the status of the job (quering the CE)
# Parameter: the jobid of the job ($1)
# Return: 0 if "DONE-OK"; 1 if job is finished badly (ABORTED|CANCELLED|DONE-FAILE); 2 otherwise
function is_finished()
{
	extract_status "$1"

  my_echo "Job status is: <${JOBSTATUS}>"

  # ... exit if it is Aborted
  if [[ "$JOBSTATUS" == ABORTED ]]; then
    debug "Job was Aborted !"
    return 1
  fi

  # ... or Cancelled
  if [[ "$JOBSTATUS" == CANCELLED ]]; then
    debug "The job has been (unexpectedly) cancelled !"
    return 1
  fi

  # ... or Failed
  if [[ "$JOBSTATUS" == DONE-FAILED ]]; then
    debug "The job finished with failure !"
    return 1
  fi

  # ... go to the next step if it is a success
  if [[ "$JOBSTATUS" == DONE-OK ]]; then
    debug "Job finished OK!"
    return 0
  fi

  return 2
}

# Wait until given job is done or the limit $NUM_STATUS_RETRIEVALS is reached
# Parameter: the JOBID ($1)
# Return: 0 if job finishes successfully; 1 otherwise
function wait_until_job_finishes()
{
  i=1

	is_finished "$1"

	ST=$?

  while [ $ST -eq 2 ]
  do

    if [ $i -ge $NUM_STATUS_RETRIEVALS ]; then
      exit_timeout $JOBID
    fi

		my_echo ""  
    my_echo "sleeping for $SLEEP_TIME seconds (run $i/$NUM_STATUS_RETRIEVALS) ..."
    sleep $SLEEP_TIME

    ((i++))
	
		is_finished "$1"
		ST=$?

  done
	
	if [ $ST -eq 0 ] ; then
		return 0
	else
		return 1
	fi

}
