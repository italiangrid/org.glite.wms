#!/bin/sh

###############################################################################
#
# A glite-wms-job-status command test
#
# Test --version option
# Test a simple status
# Test option --config
# Test option --output
# Test option --logfile
# Test the --input option
#
# BEWARE: Test these options requires certain indexing capabilities to be enabled on the LB server!
#  Test the --exclude option (exclude Waiting,  Done, Cleared and Aborted jobs)
#  Test the --status option (look for Waiting job)
#  Test the --user-tag option
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare "test glite-wms-job-status command"  $@

fail=0

START=$(date +%H:%M)
COMMAND=glite-wms-job-status
message "Check if command $COMMAND exists"
run_command "which $COMMAND" 1 "$COMMAND is not in the PATH"

# Test --version option
message ""
message "Test --version option"
run_command "$COMMAND --version" 0
if [[ $? -eq 0 ]] ; then
  verbose "Check the output command"
  run_command "grep \"User Interface version\"  $CMDOUT" 0
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

verbose "Submit a job"
run_command "glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg $JDLFILE" 1 "Submission failed"
JOBID=$OUTPUT

message ""
message "Test a simple status..."
run_command "$COMMAND $JOBID" 0
if [[ $? -eq 1 ]] ; then
	fail=$(($fail+1))
	message "Test fails"
else	
	message "Test success"
fi

message ""
message "Test option --config"
run_command "$COMMAND --config $CONFIG_FILE $JOBID" 0
if [[ $? -eq 1 ]] ; then
  fail=$(($fail+1))
  message "Test fails"
else  
  message "Test success"
fi

message ""
message "Test option --output"
run_command "$COMMAND --output $OUTPUTFILE --config $CONFIG_FILE $JOBID" 0
if [[ $? -eq 1 ]] ; then
  fail=$(($fail+1))
  message "Command fails"
else
  verbose "Check the output"
  run_command "cat $OUTPUTFILE" 0
  if [[ $? -eq 1 ]] ; then
      fail=$(($fail+1))
      message "Test fails. Output file not created"
    else
      message "Test success"
  fi
fi
remove $OUTPUTFILE

message ""
message "Test option --logfile"
run_command "$COMMAND --logfile $LOGFILE --config $CONFIG_FILE $JOBID" 0
if [[ $? -eq 1 ]] ; then
  fail=$(($fail+1))
  message "Command fails"
else
  verbose "Check the logfile"
  run_command "cat $LOGFILE" 0
  if [[ $? -eq 1 ]] ; then
      fail=$(($fail+1))
      message "Test fails. Log file not created"
    else
      message "Test success"
  fi
fi
remove $LOGFILE

verbose "Submit some jobs"
# submit some jobs
for (( i=0 ; $i < 3 ; i = $(($i + 1)) )); do
  run_command "glite-wms-job-submit $DELEGATION_OPTIONS --output $JOBIDFILE --config $CONFIG_FILE $JDLFILE" 1 "Submission failed"
done

message ""
message "Test the --input option"
run_command "$COMMAND --noint --config $CONFIG_FILE --input $JOBIDFILE" 0
if [[ $? -eq 1 ]] ; then
  fail=$(($fail+1))
  message "Test fails"
else
  message "Test success"
fi

verbose "Check if LB has indexing capabilities"
run_command "$COMMAND --config $CONFIG_FILE --noint --all" 0
if [ $? -eq 0 ] ; then
  set_requirements "\"false\""
  echo "usertags = [ type = \"test job\" ];" >> $JDLFILE
  run_command "glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --output $JOBIDFILE $JDLFILE" 1 "Submission failed"
  JOBID=`tail -1 $JOBIDFILE`

  verbose "Waiting until job arrives to the expected status.."
  get_STATUS $JOBID
  while [[ "$JOBSTATUS" != *Waiting* ]] ; do
    get_STATUS $JOBID
    run_command "sleep 5" 0
  done

  message ""
  message "Test the --exclude option (exclude Waiting,  Done, Cleared and Aborted jobs)"
  run_command "$COMMAND --config $CONFIG_FILE --all --exclude Waiting --exclude Done --exclude 8 --exclude 7" 0
	if [[ $? -eq 0 ]] ; then
  	run_command_fail "grep $JOBID $CMDOUT" 0
		if [[ $? -eq 0 ]] ; then
			message "Test success"
		else
			fail=$(($fail+1))
			message "Test fails."
		fi
	else
		fail=$(($fail+1))
		message "Command fails"
	fi

  message ""
  message "Test the --status option (look for Waiting job)"
  run_command "$COMMAND --config $CONFIG_FILE --all --status Waiting" 0
	if [[ $? -eq 0 ]] ; then
    run_command "grep $JOBID $CMDOUT" 0
    if [[ $? -eq 0 ]] ; then
      message "Test success"
    else
      fail=$(($fail+1))
      message "Test fails."
    fi
  else
    fail=$(($fail+1))
    message "Command fails"
  fi
  
	message ""
  message "Test the --user-tag option"
  run_command "$COMMAND --config $CONFIG_FILE --all --user-tag type=\"test job\"" 0
	if [[ $? -eq 0 ]] ; then
		run_command "grep $JOBID $CMDOUT" 0	
		if [[ $? -eq 0 ]] ; then
			run_command_fail "$COMMAND --config $CONFIG_FILE -all --user-tag type=\"wrong tag\"" 0	
			if [[ $? -eq 0 ]] ; then
				message "Test success"
			else	
				fail=$(($fail+1))
				message "Test fails."
			fi
		else
			fail=$(($fail+1))
			message "Test fails."
		fi
	else
		fail=$(($fail+1))
    message "Command fails"
  fi
else
  message ""
  message "BEWARE: Test other options requires certain indexing capabilities to be enabled on the LB server!"
fi

# we need to test --verbosity --to --from

# ... terminate
if [ $fail -eq 1 ] ; then
  exit_failure "$fail test(s) fail(s)"
else
  exit_success
fi

