#!/bin/sh

###############################################################################
#
# Test a complete job cycle: from submission to get-output
#
# Test submission of a normal job to an LCG-CE
# Test submission of a normal job to a CREAM CE
# or if $CE is defined submit to the given $CE
#
# We need to test submission of various type of jobs (TBD)
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh


## $1 = jobid
function joboutput()
{
	get_STATUS $1
	if [[ "$JOBSTATUS" == "Done (Success)" ]]; then
		verbose "Retrieve the output"
		run_command "glite-wms-job-output --nosubdir --dir $JOB_OUTPUT_DIR $1" 0
		if [[ $? -eq 0 ]] ; then
  		verbose "Check if the status is correct"
  		run_command_fail "grep \"Warning - JobPurging not allowed\" $CMDOUT" 0
    	if [[ $? -eq 0 ]] ; then
				sleep 5
      	get_STATUS $1
      	if [[ "$JOBSTATUS" != *Cleared* ]]; then
					verbose "Final status of job ($1) is not 'CLEARED'"
        	return 2
      	fi
    	else
      	verbose "Warning: WMS is not recognized by the LB, JobPurging not allowed!"
				return 2
    	fi
		else
			exit_message="Cannot retrieve output"
			return 1
		fi
		verbose "Check if the output files are correctly retrieved"
		if [ -f ${JOB_OUTPUT_DIR}/std.out ] && [ -f ${JOB_OUTPUT_DIR}/std.err ] ; then
			return 0
		else
			exit_message="Output files are not correctly retrieved"
			return 1
		fi
	else
		exit_message="Job finishes with status: $JOBSTATUS; cannot retrieve output"
		return 1
	fi

	return 0
}

# $1 is the required CE expression
function submitTO ()
{
	set_isbjdl $JDLFILE
	set_requirements "RegExp(\"$1\",other.GlueCEUniqueID)"
	run_command "glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg $JDLFILE" 0
	if [[ $? -eq 0 ]] ; then
		JOBID=$OUTPUT
		sleep 10
		get_CE $JOBID
		verbose "Check if it match a correct CE"
		run_command "grep $1 <<< \"$CENAME\" > /dev/null" 0
		if [ $? -eq 0 ] ; then
			wait_until_job_finishes $JOBID
			joboutput $JOBID
			return $?
		else
			exit_message="Matchmaking fails ($CENAME)"
			return 1
		fi
	else
		exit_message="Command fails"
		return 1
	fi
	return 0
}

prepare "test submission cycle" $@

fail=0

if [[ x$CE != "x" ]] ; then
	message ""
	message "Submit a normal job to $CE"
	submitTO $CE
	if [ $? -eq 1 ] ; then
		exit_failure ${exit_message}
	fi	
else	
	message ""
	message "Submit a normal job to an LCG-CE"
	submitTO "2119/jobmanager"
	if [ $? -eq 1 ] ; then
		message "Test fails. ${exit_message}"
		fail=$(($fail+1));
	else
		message "Test success"
	fi
	rm -rf ${JOB_OUTPUT_DIR}
	message ""
	message "Submit a normal job to a CREAM CE"
	submitTO "8443/cream"
	if [ $? -eq 1 ] ; then
		message "Test fails. ${exit_message}"
		fail=$(($fail+1));
	else
		message "Test success"
	fi
fi


# ... terminate
if [ $fail -eq 1 ] ; then
  exit_failure "$fail test(s) fail(s)"
else
  exit_success
fi

