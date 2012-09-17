#!/bin/sh

###############################################################################
#
# Test glite-wms-job-cancel command
#
# Test --version option
# Test --config option
# Test --output option
# Test --logfile option
# Test --input option
# Test all options together
#
# Success means that the final state of the job is "Cancelled"
#
# We try also to cancel a finished job, command should fails
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
# Version: $Id$
#
###############################################################################

function submit_job()
{
  # submit a job and set variable JOBID 
  verbose "Submit a job"
  run_command "glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg --output $JOBIDFILE $JDLFILE" 1 "Submission failed"
  JOBID=$OUTPUT
  sleep 5 # wait until job arrives to wm, needs because there is a bug in the wmproxy.
}

function check_status()
{
  i=1
  get_STATUS $JOBID

  # try to check the status 5 times (for a total of 100 seconds) 
  while [[ $JOBSTATUS != "Cancelled" ]] &&  [[ $i -le 5 ]] ; do 
    ((i=$i+1))
    sleep $(($i*5))
    get_STATUS $JOBID
  done

  # if all tries fail exit with failure 
  if [[ $JOBSTATUS != "Cancelled" ]] ; then
    if is_finished $JOBID ; then
			message "Check failed. Job ($JOBID) has not been cancelled ($JOBSTATUS)" 					
			return 1
    else
      message "WARNING: job ($JOBID) finished ($JOBSTATUS) before cancellation..."
    fi
  fi
	return 0
}

# ... startup

. $(dirname $0)/functions.sh

prepare "test command glite-wms-job-cancel" $@

fail=0

COMMAND=glite-wms-job-cancel
message "Check if command $COMMAND exists"
run_command "which $COMMAND" 1 "$COMMAND is not in the PATH"

# Test --version option
message ""
message "Test --version option"
run_command "$COMMAND --version" 0
if [[ $? -eq 0 ]] ; then 
	verbose "Check the output command"
	run_command "grep \"WMS User Interface version\" $CMDOUT" 0
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

set_longjdl $JDLFILE # use long job

message ""
message "Test --config option"
submit_job
is_finished $JOBID
if [[ $? -eq 0 ]] ; then # Cancel the job only if it is not finished
  run_command "$COMMAND --config $CONFIG_FILE --noint $JOBID" 0
	if [[ $? -eq 0 ]] ; then 
		check_status
		if [[ $? -eq 1 ]] ; then 
			fail=$(($fail+1)); 
			message "Test fails"; 
		else
			message "Test success"
		fi
	else
  	fail=$(($fail+1))
  	message "Command fails"
	fi
else
	message "WARNING: job ($JOBID) finished ($JOBSTATUS) before cancellation..."
fi
remove $JOBIDFILE # remove unused jobid file

message ""
message "Test --output option"
submit_job
is_finished $JOBID
if [[ $? -eq 0 ]]; then # Cancel the job only if it is not finished
  run_command "$COMMAND --config $CONFIG_FILE --noint --output $OUTPUTFILE $JOBID" 0
  if [[ $? -eq 0 ]] ; then 
		check_status
		if [[ $? -eq 1 ]] ; then 
			fail=$(($fail+1)) 
			message "Test fails"
		else
			verbose "Check output file:"
			run_command "cat $OUTPUTFILE" 0
			if [[ $? -eq 1 ]] ; then
				fail=$(($fail+1))
				message "Test fails. Output file not created"
			else
				message "Test success"
			fi
		fi
  else
    fail=$(($fail+1))
    message "Command fails"
  fi
else
  message "WARNING: job ($JOBID) finished ($JOBSTATUS) before cancellation..."
fi
remove $JOBIDFILE # remove unused jobid file

message ""
message "Test --logfile option"
submit_job
is_finished $JOBID
if [[ $? -eq 0 ]] ; then # Cancel the job only if it is not finished
  run_command "$COMMAND --config $CONFIG_FILE --noint --logfile $LOGFILE $JOBID" 0
  if [[ $? -eq 0 ]] ; then
    check_status
		if [[ $? -eq 1 ]] ; then 
      fail=$(($fail+1))
      message "Test fails"
    else
			verbose "Check logfile:"
			run_command "cat $LOGFILE" 0
      if [[ $? -eq 1 ]] ; then
        fail=$(($fail+1))
        message "Test fails. Logfile file not created"
      else
				message "Test success"
			fi
		fi
  else
    fail=$(($fail+1))
    message "Command fails"
  fi
else
  message "WARNING: job ($JOBID) finished ($JOBSTATUS) before cancellation..."
fi
remove $JOBIDFILE # remove unused jobid file

message ""
message "Test --input option"
# to test "input" option submit 5 jobs
i=0
while [[ $i < 5 ]] ; do
  submit_job
  ((i=$i+1))
done
run_command "$COMMAND --config $CONFIG_FILE --noint --input $JOBIDFILE" 0
if [[ $? -eq 0 ]] ; then
	for JOBID in `cat $JOBIDFILE`; do
  	if [[ "$JOBID" == "http"* ]]; then 
    	verbose "Check status of $JOBID"
    	check_status
			if [[ $? -eq 1 ]] ; then i=$(($i-1)) ; fi
  	fi
	done
else
	fail=$(($fail+1))
  message "Command fails"
fi
remove $JOBIDFILE  
if [[ $i -eq 0 ]] ; then ## all jobs fail
	fail=$(($fail+1))
  message "Test fails"
else
	message "Test success"
fi

message ""
message "Test all options together"
submit_job
is_finished $JOBID
if [[ $? -eq 0 ]] ; then
  run_command "$COMMAND --noint --input $JOBIDFILE --output $OUTPUTFILE --logfile $LOGFILE --config $CONFIG_FILE" 0
	if [[ $? -eq 0 ]] ; then
		check_status
		if [[ $? -eq 1 ]] ; then
      fail=$(($fail+1))
      message "Test fails"
    else
			verbose "Check logfile:"
			run_command "cat $LOGFILE" 0
			if [[ $? -eq 1 ]] ; then
        fail=$(($fail+1))
        message "Test fails. Logfile file not created"
      else
				verbose "Check outfile:"
				run_command "cat $OUTPUTFILE" 0
				if [[ $? -eq 1 ]] ; then
        	fail=$(($fail+1))
        	message "Test fails. Output file not created"
      	else
					message "Test success"
				fi
			fi
		fi
  else
    fail=$(($fail+1))
    message "Command fails"
  fi
else
  message "WARNING: job ($JOBID) finished ($JOBSTATUS) before cancellation..."
fi
remove $JOBIDFILE

set_jdl $JDLFILE # use a short job

message ""
message "Try to cancel a DoneOk job"
submit_job
wait_until_job_finishes $JOBID
run_command_fail "$COMMAND --noint $JOBID" 0
if [[ $? -eq 1 ]] ; then
	fail=$(($fail+1))
	message "Test fails."
else
	message "Test success"
fi

# ... terminate

if [ $fail -ne 0 ] ; then
  exit_failure "$fail test(s) fail(s)"
else
  exit_success
fi

