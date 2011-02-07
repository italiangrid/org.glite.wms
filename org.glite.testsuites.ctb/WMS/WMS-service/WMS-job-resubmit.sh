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

# $1 if the jdl file
# set $JOBID
function submit ()
{
	run_command glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg $1
	JOBID=$OUTPUT
	wait_until_job_finishes $JOBID
	return $?	
}

prepare "test job resubmission" $@

message ""
message "Try a shallow resubmission"
set_shallow_jdl $JDLFILE
submit $JDLFILE
if [[ "$JOBSTATUS" == *Aborted* ]] ; then # job is aborted
	run_command "glite-wms-job-logging-info --event RESUBMISSION $JOBID | grep -c SHALLOW"
	if [[ $OUTPUT -ne 2 ]] ; then 
		exit_failure "Job $JOBID hasn't be correctly resubmitted"
	fi
else
	exit_warning "Something goes wrong: job final status is $JOBSTATUS"
fi

message ""
message "Try a deep resubmission"
set_deep_jdl $JDLFILE
submit $JDLFILE
if [[ "$JOBSTATUS" == *Aborted* ]] ; then # job is aborted
	run_command "glite-wms-job-logging-info -v 2 --event RESUBMISSION $JOBID | grep -c 'token was grabbed'"
	if [[ $OUTPUT -ne 2 ]] ; then # There must be 2 resubmissions
		exit_failure "Job $JOBID hasn't be correctly resubmitted"
	else # Check if resubmissions have been triggered from epilogue failures
		run_command "glite-wms-job-logging-info -v 2 --event Done $JOBID | grep -B 1 'Source[[:space:]]*=[[:space:]]*LogMonitor' | grep -i -c 'Epilogue failed with error 1'"
		if [[ $OUTPUT -ne 3 ]] ; then # if not look at failure messages
			glite-wms-job-logging-info -v 2 --event Done $JOBID | grep 'Cannot take token' &> /dev/null
			if [[ $? -ne 0 ]] ; then # If error message is "Cannot take token" something goes wrong in the WMS
				exit_failure "Probably the token for job $JOBID has not been recreated in WMS. Check it."
			else
				exit_failure "Job $JOBID fails for an unexpected reason. Check it."
			fi
		fi
	fi
else
	exit_warning "Something goes wrong: job final status is $JOBSTATUS"
fi


# All goes well

exit_success
