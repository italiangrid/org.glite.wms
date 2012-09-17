#!/bin/sh

###############################################################################
#
# A glite-wms-job-logging-info command test
#
# Test --version option
# Test a simple logging info...
# Test option --config
# Test option --output
# Test option --logfile
# Test the --input option
# Test the --to option
# Test the --from option
# Test the --event option (show only ACCEPTED events)
# Test the --exclude option (exclude ACCEPTED events)
# Test all the options together (extract only EnQueued events)
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare "test glite-wms-job-logging-info command"  $@

fail=0

START=$(date +%H:%M)
COMMAND=glite-wms-job-logging-info
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
run_command "glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg --output $JOBIDFILE $JDLFILE" 1 "Submission failed"
JOBID=$OUTPUT

message ""
message "Test a simple logging info..."
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


# with file with more than 2 jobs it doesn't work for a bug
remove $JOBIDFILE
verbose "Submit some jobs"
run_command "glite-wms-job-submit $DELEGATION_OPTIONS --output $JOBIDFILE --config $CONFIG_FILE $JDLFILE" 1 "Submission failed"
run_command "glite-wms-job-submit $DELEGATION_OPTIONS --output $JOBIDFILE --config $CONFIG_FILE $JDLFILE" 1 "Submission failed"

message ""
message "Test the --input option"
run_command "$COMMAND --noint --config $CONFIG_FILE --input $JOBIDFILE" 0
if [[ $? -eq 1 ]] ; then
  fail=$(($fail+1))
  message "Test fails"
else
  message "Test success"
fi

run_command "sleep 60"
NOW=$(date +%H:%M)        # format date, preserve leading zeros (e.g. 07:07)

message ""
message "Test the --to option"
run_command "$COMMAND --config $CONFIG_FILE --to $NOW $JOBID" 0	# this test fails on gLite UI 3.2.2
if [[ $? -eq 1 ]] ; then
  fail=$(($fail+1))
  message "Test fails (Is it a glite UI 3.2.2?)"
else
  message "Test success"
fi

message ""
message "Test the --from option"
run_command "$COMMAND --config $CONFIG_FILE --from $START $JOBID" 0
if [[ $? -eq 1 ]] ; then
  fail=$(($fail+1))
  message "Test fails"
else
  message "Test success"
fi

message ""
message "Test the --event option (show only ACCEPTED events)"
run_command "$COMMAND --config $CONFIG_FILE --event ACCEPTED $JOBID | grep \"Event: Accepted\"" 0
if [[ $? -eq 1 ]] ; then
  fail=$(($fail+1))
  message "Test fails"
else
  message "Test success"
fi

message ""
message "Test the --exclude option (exclude ACCEPTED events)"
run_command_fail "$COMMAND --config $CONFIG_FILE --exclude ACCEPTED $JOBID | grep \"Event: Accepted\"" 0
if [[ $? -eq 1 ]] ; then
  fail=$(($fail+1))
  message "Test fails"
else
  message "Test success"
fi

message ""
message "Test all the options together (extract only EnQueued events)"
run_command "$COMMAND --noint --input $JOBIDFILE --output $OUTPUTFILE --logfile $LOGFILE --config $CONFIG_FILE --verbosity 3 --from $START --to $NOW --event EnQueued" 0
if [[ $? -eq 1 ]] ; then
  fail=$(($fail+1))
  message "Command fails"
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
remove $LOGFILE
remove $OUTPUTFILE

# ... terminate
if [ $fail -eq 1 ] ; then
  exit_failure "$fail test(s) fail(s)"
else
  exit_success
fi

