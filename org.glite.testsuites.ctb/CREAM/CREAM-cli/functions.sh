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
# The test suite requires example.jdl and test.conf to be present
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
  echo -e " --> $@"
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

# Remove tmp directory $MYTMPDIR if not $DEBUG
function cleanup()
{

	if [ $DEBUG -eq 0 ] ; then
  	my_echo "cleaning up $MYTMPDIR ..."
  	[[ -d "$MYTMPDIR" ]] && rm -rf $MYTMPDIR
	fi

	return 0
}

# Execute the given command 
# Parameter: the command ($@) 
# Set: COM_OUTPUT with the command output
# Return: 0 if success, 1 otherwise
function run_command()
{
	unset COM_OUTPUT
  my_echo ""
  my_echo "[" $(date +%H:%M:%S) "] run: " $@

  COM_OUTPUT=$(eval $@ 2>&1)

  if [ $? -ne 0 ]; then
		my_echo "Command FAILED"
    debug "Command output is: ${COM_OUTPUT}"
    return 1
  fi

	debug "Command runs succesfully. Output is: \n${COM_OUTPUT}"
  return 0 
}

# ... print usage message and exit
function usage()
{
  echo "Usage: "
  echo ""
  echo $(basename $0) "[-d -h]"
  echo ""
  echo " --debug (-d) Print debug messages"
  echo " --help  (-h) Print this message"
  echo ""
  exit 0
}

# Prepare the enviroment
# Set: MYTMPDIR (to a working temporary directory)
# Set: ENDPOINT (extract the ENDPOINT from the CREAM variable defined in test.conf)
# Set: QUEUE (extract the QUEUE from the CREAM variable defined in test.conf)
# Set: START_TIME (the time when the test start)
# Set: LOGFILE (pointing to a temporary log file)
function prepare()
{
	# Sources global variables (test.conf is REQUIRED)
	# defien defaults values
	DEBUG=0
	NUM_STATUS_RETRIEVALS=40
	SLEEP_TIME=5
	JDLFILE=$(dirname $0)/example.jdl
	[[ -f test.conf ]] || exit_failure "Internal ERROR! Could not find the required 'test.conf' file!"
	source test.conf

	# check if debug is required (it overwrites the tets.conf file)
  if [ "$1" == "-d" ] || [ "$1" == "--debug" ]; then
    DEBUG=1
  fi

	# print usage message in required
  if [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
    usage
    exit 0
  fi
  
  my_echo "++++++++++++++++++++++++++++++++++++++++++++"
  my_echo "+ Test of CREAM-CE  command line interface +"
  my_echo "++++++++++++++++++++++++++++++++++++++++++++"

  START_TIME=$(date +%H:%M:%S)
  my_echo "Test starts at: $START_TIME"

	# The JDLFILE is required!
  [[ -f "$JDLFILE" ]] || exit_failure "Internal ERROR! Could not find example jdl file $JDLFILE"

  # ... create temporary directory
  MYTMPDIR=/tmp/cream-cli-test-$(id -un)-$$
  mkdir $MYTMPDIR || exit_failure "Internal ERROR! Could not create temporary directory $MYTMPDIR"

	# Set some auxiliar variables 
  ENDPOINT=`echo $CREAM | awk -F'/' '{print $1}'`
	QUEUE=`echo $CREAM | awk -F'-' '{print $NF}'`
	LOGFILE=$MYTMPDIR/file.log

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
  JOBSTATUS=$(${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-status -n -L 0 "$1" | awk -F'[\\\[\\\]]' '/Status/ {print $2}' - 2>/dev/null)
  debug "Job status is: $JOBSTATUS"
  if [ -z "$JOBSTATUS" ]; then
    exit_failure "ERROR: could not determine Job STATUS!"
  fi
	return 0
}

# Check the status of the job (quering the CE)
# Parameter: the jobid of the job ($1)
# Return: 0 if "DONE-OK"; 1 if job is finished badly (ABORTED|CANCELLED|DONE-FAILED); 2 otherwise
function is_finished()
{
	extract_status "$1"

  my_echo "Job status is: <${JOBSTATUS}>"

  # ... exit if it is Aborted
  if [[ "$JOBSTATUS" == ABORTED ]]; then
    debug "Job is Aborted !"
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

# Wait until given job is finished or the limit $NUM_STATUS_RETRIEVALS is reached
# Set: JOBID of the finished job
# Return: 0 if job finishes successfully; 1 otherwise
function wait_until_job_finishes()
{
# define a short jdl
  printf "[
JobType = \"Normal\";
Executable = \"/bin/hostname\";
StdOuput=\"out.txt\";
StdError=\"err.txt\";
]
" > ${MYTMPDIR}/short.jdl

	i=1

  run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-submit -a -r $CREAM ${MYTMPDIR}/short.jdl
  if [ $? -ne 0 ]; then
    exit_failure ${COM_OUTPUT}
  fi

  extract_jobid ${COM_OUTPUT}
  debug "Submitted job: $JOBID"

  is_finished "$JOBID"

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
	
		is_finished "$JOBID"
		ST=$?

  done
	
	if [ $ST -eq 0 ] ; then
		return 0 # DONE_OK 
	else
		return 1 # ABORTED|CANCELLED|DONE-FAILED
	fi

}

# Returns a new delegation name
function new_delegation_id(){
  echo DelegateId_`hostname`_`date +%s`
}

# Wait until given job is running
# Set: JOBID of the running job
function wait_until_job_runs()
{
# define a long jdl
  printf "[
JobType = \"Normal\";
Executable = \"/bin/sleep\";
Arguments = \"600\";
StdOuput=\"out.txt\";
StdError=\"err.txt\";
]
" > ${MYTMPDIR}/long_sleep.jdl

  run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-submit -a -r $CREAM ${MYTMPDIR}/long_sleep.jdl
  if [ $? -ne 0 ]; then
    return 1
  fi

  extract_jobid ${COM_OUTPUT}
  debug "Submitted job: $JOBID"

  ok=0
  while [ "$ok" == "0" ]; do

    my_echo "Waiting for the running job (10s)"
    sleep 10

    extract_status $JOBID

    if [ "$JOBSTATUS" == "RUNNING" -o "$JOBSTATUS" == "REALLY-RUNNING" ]; then
      ok=1
    fi
    if [ "$JOBSTATUS" == "DONE-FAILED" -o "$JOBSTATUS" == "ABORTED" ]; then
      COM_OUTPUT="Cannot run the job (${JOBSTATUS})"
      return 1
    fi
  done

return 0
}

# Extract the last event ID
# Parameter: the output of a event query command ($@)
# Set: LASTEVENTID with the event ID
# Return: 0 or exit with failure
function get_last_event_id ()
{
  LASTEVENTID=$(echo $@ | grep -Eo 'EventID=\[[0-9]+\]' | tail -1 | grep -Eo [0-9]+)
  debug "Job status is: $LASTEVENTID"
  if [ -z "$LASTEVENTID" ]; then
    exit_failure "ERROR: could not determine last event ID!"
  fi
	return 0
}
