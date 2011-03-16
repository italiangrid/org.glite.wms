#!/bin/sh

###############################################################################
#
# Test resubmission: shallow and deep.
#
# Test shallow resubmission
# Test deep resubmission
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh


# define a jdl which trigger 1 shallow resubmission and save it in $1
function set_shallow_jdl()
{
  remove $1
  echo "Executable = \"/bin/hostname\";" >> $1
  echo "StdOutput  = \"std.out\";" >> $1
  echo "StdError   = \"std.err\";" >> $1
  echo "OutputSandbox = {\"std.out\",\"std.err\"};" >> $1
  echo "ShallowRetryCount = 2; " >> $1
	echo "Prologue = \"/bin/false\";" >> $1
  return 0
}

# define a jdl which trigger 1 deep resubmission and save it in $1
function set_deep_jdl()
{
  remove $1
  echo "Executable = \"/bin/hostname\";" >> $1
  echo "StdOutput  = \"std.out\";" >> $1
  echo "StdError   = \"std.err\";" >> $1
  echo "OutputSandbox = {\"std.out\",\"std.err\"};" >> $1
  echo "RetryCount = 2; " >> $1
	echo "Epilogue = \"/bin/false\";" >> $1
  return 0
}

# $1 is the jdl file
# set $JOBID
function submit ()
{
	run_command "glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg $1" 1 "Submission failed"
	JOBID=$OUTPUT
	wait_until_job_finishes $JOBID
	return $?	
}

fail=0

prepare "test job resubmission" $@

message ""
message "Try a shallow resubmission"
set_shallow_jdl $JDLFILE
submit $JDLFILE
if [[ "$JOBSTATUS" == *Aborted* ]] ; then # job is aborted
	run_command "glite-wms-job-logging-info --event RESUBMISSION $JOBID | grep -c SHALLOW" 0
	if [[ $? -eq 0 ]] ; then
		if [[ $OUTPUT -ne 2 ]] ; then 
			message "Test fails. Job $JOBID hasn't be correctly resubmitted"
			fail=$(($fail+1));
		else
			message "Test success"
		fi
	else
		message "Test fails. Cannot retrieve job's logging-info"	
		fail=$(($fail+1))
	fi
else
	fail=$(($fail+1));
	message "Test fails. Something goes wrong: job final status is $JOBSTATUS"
fi

message ""
message "Try a deep resubmission"
set_deep_jdl $JDLFILE
submit $JDLFILE
if [[ "$JOBSTATUS" == *Aborted* ]] ; then # job is aborted
	run_command "glite-wms-job-logging-info -v 2 --event RESUBMISSION $JOBID | grep -c 'token was grabbed'" 0
	if [[ $? -eq 0 ]] ; then
		if [[ $OUTPUT -ne 2 ]] ; then # There must be 2 resubmissions
			fail=$(($fail+1))
			message "Test fails. Job $JOBID hasn't be correctly resubmitted"
		else # Check if resubmissions have been triggered from epilogue failures
			run_command "glite-wms-job-logging-info -v 2 --event Done $JOBID | grep -B 1 'Source[[:space:]]*=[[:space:]]*LogMonitor' | grep -i -c 'Epilogue failed with error 1'" 0
			if [[ $? -eq 0 ]] ; then
				if [[ $OUTPUT -ne 3 ]] ; then # if not look at failure messages
					run_command "glite-wms-job-logging-info -v 2 --event Done $JOBID | grep 'Cannot take token' &> /dev/null"
					if [[ $? -ne 0 ]] ; then # If error message is "Cannot take token" something goes wrong in the WMS
						message "Test fails. Probably the token for job $JOBID has not been recreated in WMS. Check it."
						fail=$(($fail+1))
					else
						message "Warning. Job $JOBID has been resubmitted for un unexpected reason. Check it."
						fail=$(($fail+1))
					fi
				else
					message "Test success"
				fi
			else
				message "Test fails. Job $JOBID fails for an unexpected reason. Check it."
				fail=$(($fail+1))
			fi
		fi
	else
		message "Test fails. Job $JOBID has not been resubmitted"
		fail=$(($fail+1))
	fi
else
	message "Test fails. Something goes wrong: job final status is $JOBSTATUS"
	fail=$(($fail+1))
fi

# ... terminate

if [ $fail -ne 0 ] ; then
  exit_failure "$fail test(s) fail(s)"
else
  exit_success
fi

