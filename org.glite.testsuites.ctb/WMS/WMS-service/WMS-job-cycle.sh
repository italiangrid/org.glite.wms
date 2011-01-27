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
		run_command glite-wms-job-output --dir $JOB_OUTPUT_DIR $1 
  	verbose "Check if the status is correct"
  	grep "\"Warning - JobPurging not allowed\"" <<< $OUTPUT
    if [[ $? -eq 1 ]] ; then
			sleep 5
      get_STATUS $1
      if [[ "$JOBSTATUS" != *Cleared* ]]; then
        message "WARNING Job $1 not cleared!"
				exit_message="Final status of job is not 'CLEARED'"
        return 2
      fi
    else
      verbose "Warning: WMS is not recognized by the LB, JobPurging not allowed!"
    fi
		verbose "Check if the output files are correctly retrieved"
		if [ -f ${JOB_OUTPUT_DIR}/std.out ] && [ -f ${JOB_OUTPUT_DIR}/std.err ] ; then
			return 0
		else
			exit_message="Output files are not correctly retrieved"
			return 1
		fi
	else
		message "WARNING Job finishes with status: $JOBSTATUS; cannot retrieve output"
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
	run_command glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg $JDLFILE
	JOBID=$OUTPUT
	sleep 10
	get_CE $JOBID
	verbose "Check if it match a correct CE"
	grep $1 <<< "$CENAME" > /dev/null
	if [ $? -eq 0 ] ; then
		wait_until_job_finishes $JOBID
		joboutput $JOBID
		return $?
	else
		exit_message="Matchmaking fails"
		message "Matching CE is $CENAME"
		return 1
	fi
}

prepare "test submission cycle" $@

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
		exit_failure ${exit_message}
	fi
	rm -rf ${JOB_OUTPUT_DIR}
	message ""
	message "Submit a normal job to a CREAM CE"
	submitTO "8443/cream"
	if [ $? -eq 1 ] ; then
		exit_failure ${exit_message}
	fi
fi



# ... terminate
if [ $? -eq 2 ] ; then
	exit_warning ${exit_message}
else
	exit_success
fi
