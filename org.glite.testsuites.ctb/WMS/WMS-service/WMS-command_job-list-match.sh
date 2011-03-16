#!/bin/sh

#############################################################################################
#
# A glite-wms-job-list-match test.
#
# Test these options: --logfile --output --endpoint --rank -> success if command returns 0
# Test a failure matching (requirements = false)           -> success if no matching is found
# Test a restricted matching (only Cream CEs)              -> success if none LCG CE matches
# Test delegation proxy (delegate before the mm)           -> success if command returns 0
#
# NB: it is not easy to check if the output of a list-match is correct
#     you can easily test customize requirements using set_requirements function
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
# Version: $Id$
#
#############################################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare "test glite-wms-job-list-match command." $@

fail=0

COMMAND=glite-wms-job-list-match
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

# Test option --endpoint
message ""
message "Test --endpoint option"
run_command "$COMMAND $DELEGATION_OPTIONS --endpoint https://${WMS}:7443/glite_wms_wmproxy_server $JDLFILE" 0
if [[ $? -ne 0 ]] ; then
	fail=$(($fail+1));
	message "Test fails";
else
	message "Test success"
fi

# Test option --output
message ""
message "Test --output option"
run_command "$COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --output $OUTPUTFILE $JDLFILE"
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

# Test option --logfile
message ""
message "Test --logfile option"
run_command "$COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --logfile $LOGFILE $JDLFILE" 0
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



# Test all options together
message ""
message "Test all options together"
run_command "$COMMAND $DELEGATION_OPTIONS --noint --logfile $LOGFILE --output $OUTPUTFILE --endpoint https://${WMS}:7443/glite_wms_wmproxy_server $JDLFILE" 0
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

# use option --rank
message ""
message "Test --rank option"
run_command "$COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --rank $JDLFILE" 0
if [[ $? -ne 0 ]] ; then
  fail=$(($fail+1));
  message "Test fails";
else
  message "Test success"
fi

## try a failure matching
message ""
message "Try a failure matching (Requirements == false)"
set_requirements "\"false\""
run_command "$COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE $JDLFILE" 0
if [[ $? -ne 0 ]] ; then
  fail=$(($fail+1))
  message "Command fails"
else
	verbose "Check output"
	run_command "grep \"No Computing Element matching your job requirements has been found!\" $CMDOUT" 0
	if [[ $? -ne 0 ]] ; then
		fail=$(($fail+1))
		message "Test fails"
	else
		message "Test success"
	fi
fi

## try a restricted requirements
message ""
message "Match only Cream's CE"
set_requirements "regexp(\"8443/cream-\", other.GlueCEUniqueID)"
run_command "$COMMAND $DELEGATION_OPTIONS --debug --config $CONFIG_FILE $JDLFILE" 0
if [[ $? -ne 0 ]] ; then
  fail=$(($fail+1));
  message "Command fails";
else
  verbose "Look for LCG-CE in the command's output"
  run_command_fail "grep \"2118/jobmanager-\" $CMDOUT" 0
  if [[ $? -ne 0 ]] ; then
    fail=$(($fail+1));
    message "Test fails";
  else
    message "Test success"
  fi
fi

## use a previouly delegated proxy
message ""
message "Use a previouly delegated proxy"
define_delegation
set_jdl $JDLFILE
run_command "$COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --rank $JDLFILE" 0
if [[ $? -ne 0 ]] ; then
  fail=$(($fail+1));
  message "Test fails";
else
  message "Test success"
fi

# ... terminate

if [ $fail -eq 1 ] ; then
  exit_failure "$fail test(s) fail(s)"
else
  exit_success
fi

