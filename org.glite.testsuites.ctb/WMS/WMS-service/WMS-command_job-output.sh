#!/bin/sh

###############################################################################
#
# Test glite-wms-job-output command.
#
# Test --version option
# Test --logfile option
# Test --dir option
# Test --nosubdir option
# Test --list-only option
# Test --nopurge option
# Test --input option
# 
# Success means that the status is "CLEARED" (otherwise a WARNING is produced)
# and that output files std.out and std.err are correctly retrieved
#
# Try to retrieve output of a Cleared job (command should failed)
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

function checkstatus()
{
run_command "sleep 5" 0
verbose "Check if the status is correct"
run_command "grep \"Warning - JobPurging not allowed\" $TMPFILE" 0
if [[ $? -eq 1 ]] ; then
  get_STATUS $1
  if [[ "$JOBSTATUS" != *Cleared* ]]; then
    message "WARNING Job $1 not cleared!"
    return 1
  fi
else
  verbose "Warning: WMS is not recognized by the LB, JobPurging not allowed!"
fi 

return 0
}

function checkoutput() 
{
verbose "Check if the output files are correctly retrieved"
run_command "grep -A 2 $1 $TMPFILE | grep -A 1 'have been successfully retrieved' | grep -v 'have been successfully retrieved'" 0
if [[ $? -eq 0 ]] ; then
	dir=$OUTPUT
	verbose "Look if in $dir there are the output files std.out and std.err"
	run_command "ls ${dir}/std.out" 0
	if [[ $? -eq 0 ]] ; then	
		run_command "ls ${dir}/std.err" 0
		if [[ $? -eq 0 ]] ; then
			remove ${dir}/std.err
			remove ${dir}/std.out
			rmdir ${dir}
			return 0
		else
			message "Cannot find std.err"
			return 1
		fi
	else
		message "Cannot find std.out"
		return 1
	fi
else
	message "Output files are not retrieved"
	return 1
fi

}

function check()
{
checkstatus $1
if [[ $? -eq 0 ]] ; then
  checkoutput $1
	if [[ $? -eq 0 ]] ; then
		return 0
	fi
else
	return 2 # Just a warning
fi
return 1
}

prepare "test glite-wms-job-output command" $@

fail=0

COMMAND=glite-wms-job-output
message "Check if command $COMMAND exists"
run_command "which $COMMAND" 1 "$COMMAND is not in the PATH"

# Test --version option
message ""
message "Test --version option"
run_command "$COMMAND --version" 0
if [[ $? -eq 0 ]] ; then
  verbose "Check the output command"
  run_command "grep \"WMS User Interface version\"  $CMDOUT" 0
  if [[ $? -eq 0 ]] ; then
    verbose "We are testing $OUTPUT"
    message "Test success"
  else
    verbose "Version number not found ($OUTPUT)"
  fi
else
  fail=$(($fail+1))
  message "Command fails"
fi

# submit some jobs
for (( i=0 ; $i < 6 ; i = $(($i + 1)) )); do
	run_command "glite-wms-job-submit $DELEGATION_OPTIONS  --config $CONFIG_FILE --nomsg $JDLFILE" 1 "Submission failed"
	JOBID[$i]=$OUTPUT
done

wait_until_job_finishes ${JOBID[0]}
# Test option --logfile
message ""
message "Test --logfile option"
run_command "$COMMAND --logfile $LOGFILE ${JOBID[0]}" 0
if [[ $? -eq 0 ]] ; then
	echo "$OUTPUT" > $TMPFILE
	check ${JOBID[0]}
	if [[ $? -ne 1 ]] ; then
		verbose "Check the logfile"
		run_command "cat $LOGFILE" 0
		if [[ $? -eq 0 ]] ; then
			message "Test success"
		else
			fail=$(($fail+1))
			message "Test fails. Log file not created"
		fi
	else
		fail=$(($fail+1))
		message "Test fails."
	fi
else
	fail=$(($fail+1))
	message "Command fails"
fi
			
message ""
message "Try to purge a Cleared job"
run_command_fail "$COMMAND --noint ${JOBID[0]}" 0
if [[ $? -eq 0 ]] ; then
	message "Test success"
else
	fail=$(($fail+1))
  message "Test fails."
fi


wait_until_job_finishes ${JOBID[1]}
message ""
message "Test --dir option"
run_command "$COMMAND --dir ${JOB_OUTPUT_DIR} ${JOBID[1]}" 0
if [[ $? -eq 0 ]] ; then
	echo "$OUTPUT" > $TMPFILE
  check ${JOBID[1]}
  if [[ $? -ne 1 ]] ; then
  	message "Test success"
  else
    fail=$(($fail+1))
    message "Test fails."
  fi
else
  fail=$(($fail+1))
  message "Command fails"
fi

wait_until_job_finishes ${JOBID[2]}
message ""
message "Test --nosubdir option"
run_command "$COMMAND --noint --nosubdir --dir ${JOB_OUTPUT_DIR} ${JOBID[2]}" 0
if [[ $? -eq 0 ]] ; then
	echo "$OUTPUT" > $TMPFILE
  check ${JOBID[2]}
  if [[ $? -ne 1 ]] ; then
    message "Test success"
  else
    fail=$(($fail+1))
    message "Test fails."
  fi
else
  fail=$(($fail+1))
  message "Command fails"
fi

wait_until_job_finishes ${JOBID[3]}
message ""
message "Test --list-only option"
run_command "$COMMAND --list-only ${JOBID[3]}" 0
if [[ $? -eq 0 ]] ; then
	echo "$OUTPUT" > $TMPFILE
  run_command "grep std.out $TMPFILE" 0
	if [[ $? -eq 0 ]] ; then
		run_command "grep std.err $TMPFILE" 0 
		if [[ $? -eq 0 ]] ; then
    	message "Test success"
		else
			fail=$(($fail+1))
			message "Test fails. std.err is not listed"
		fi
  else
    fail=$(($fail+1))
    message "Test fails. std.out is not listed."
  fi
else
  fail=$(($fail+1))
  message "Command fails"
fi

message ""
message "Test --nopurge option"
run_command "$COMMAND --nopurge ${JOBID[3]}"
if [[ $? -eq 0 ]] ; then
	get_STATUS ${JOBID[3]}
	if [[ "$JOBSTATUS" != *Cleared* ]]; then
		verbose "Try again to retrieve the output"
		run_command "$COMMAND --noint --nopurge ${JOBID[3]}" 0
		if [[ $? -eq 0 ]] ; then
			message "Test success"
		else
			fail=$(($fail+1))
			message "Test fails."
		fi
	else	
		fail=$(($fail+1))
		message "Test fails. Job ${JOBID[3]} has been purged"
	fi
else
  fail=$(($fail+1))
  message "Command fails"
fi

wait_until_job_finishes ${JOBID[4]}
wait_until_job_finishes ${JOBID[5]}
message ""
message "Test --input option"
echo ${JOBID[3]} > $JOBIDFILE
echo ${JOBID[4]} >> $JOBIDFILE
echo ${JOBID[5]} >> $JOBIDFILE
run_command "$COMMAND --noint --input $JOBIDFILE" 0
if [[ $? -eq 0 ]] ; then
	echo "$OUTPUT" > $TMPFILE
	check ${JOBID[3]}
	if [[ $? -ne 1 ]] ; then
		check ${JOBID[4]}
		if [[ $? -ne 1 ]] ; then
			check ${JOBID[5]}
			if [[ $? -ne 1 ]] ; then
  			message "Test success"
			else
				fail=$(($fail+1))
				message "Test fails. Output is not correct for ${JOBID[5]}"
			fi
		else
			fail=$(($fail+1))
			message "Test fails. Output is not correct for ${JOBID[4]}"
		fi
	else
		fail=$(($fail+1))
		message "Test fails. Output is not correct for ${JOBID[3]}"
	fi
else
  fail=$(($fail+1))
  message "Test fails."
fi

# ... terminate
if [ $fail -eq 1 ] ; then
  exit_failure "$fail test(s) fail(s)"
else
  exit_success
fi
