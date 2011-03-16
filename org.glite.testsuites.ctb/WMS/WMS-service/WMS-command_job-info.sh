#!/bin/sh

###############################################################################
#
# A glite-wms-job-info command test
#
# Test --version option
# Test these options for all the command: --jdl-original --jdl --proxy --delegationid
#   Test --input option
#   Test --config option
#   Test --output option
#   Test --logfile option
#   Test --endpoint option
#   Test all options toghether
# Check the edg_jobid parameter in the registered jdl"
# Check the expiration time of the delegation"
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
# Version: $Id$
#
###############################################################################

# ... startup, force proxy delegation

. $(dirname $0)/functions.sh

prepare "test glite-wms-job-info command" $@

fail=0

COMMAND=glite-wms-job-info
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

# ... submit a job

define_delegation
verbose "Submit a job"
run_command "glite-wms-job-submit $DELEGATION_OPTIONS -c $CONFIG_FILE --nomsg --output $JOBIDFILE $JDLFILE" 1 "Submission failed"
JOBID=$OUTPUT

# test these manadatory options --jdl-original --jdl --proxy 
# combined with differents extra options

for opt in --jdl-original --jdl --proxy ; do

  message ""
  message "Test $COMMAND $opt with --input option"
  run_command "$COMMAND --input $JOBIDFILE $opt" 0
  if [[ $? -eq 1 ]] ; then
    fail=$(($fail+1))
    message "Command fails"
  else
    message "Test success"
  fi

  message ""
  message "Test $COMMAND $opt with --config option"
  run_command "$COMMAND --config $CONFIG_FILE $opt $JOBID" 0
	if [[ $? -eq 1 ]] ; then
    fail=$(($fail+1))
    message "Command fails"
  else
    message "Test success"
  fi

  message ""
  message "Test $COMMAND $opt with --output option"
  run_command "$COMMAND --output $OUTPUTFILE $opt $JOBID" 0
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

  message ""
  message "Test $COMMAND $opt with --logfile option"
  run_command "$COMMAND --logfile $LOGFILE $opt $JOBID" 0
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

  message ""
  message "Test $COMMAND $opt with --endpoint option"
  run_command "$COMMAND --endpoint https://${WMS}:7443/glite_wms_wmproxy_server $opt $JOBID" 0
	if [[ $? -eq 1 ]] ; then
    fail=$(($fail+1))
    message "Command fails"
  else
    message "Test success"
  fi


  message ""
  message "Test $COMMAND $opt with all options"
  run_command "$COMMAND --noint --debug --input $JOBIDFILE --config $CONFIG_FILE  --output $OUTPUTFILE  --logfile $LOGFILE $opt" 0
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

done

## test --delegationid mandatory option"

message ""
message "Test $COMMAND --delegationid with --config option"
run_command "$COMMAND --config $CONFIG_FILE $DELEGATION_OPTIONS" 0 
if [[ $? -eq 1 ]] ; then
  fail=$(($fail+1))
  message "Test fails"
else
  message "Test success"
fi

message ""
message "Test $COMMAND --delegationid with --output option"
run_command "$COMMAND --output $OUTPUTFILE --config $CONFIG_FILE $DELEGATION_OPTIONS" 0
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

message ""
message "Test $COMMAND --delegationid with --logfile option"
run_command "$COMMAND --logfile $LOGFILE --config $CONFIG_FILE $DELEGATION_OPTIONS" 0
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

message ""
message "Test $COMMAND --delegationid with with --endpoint option"
run_command "$COMMAND --endpoint https://${WMS}:7443/glite_wms_wmproxy_server $DELEGATION_OPTIONS" 0 
if [[ $? -eq 1 ]] ; then
  fail=$(($fail+1))
  message "Test fails"
else
  message "Test success"
fi

message ""
message "Test $COMMAND --delegationid with all options"
run_command "$COMMAND --noint --debug --config $CONFIG_FILE --output $OUTPUTFILE --logfile $LOGFILE --endpoint https://${WMS}:7443/glite_wms_wmproxy_server $DELEGATION_OPTIONS" 0
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

## some ad hoc tests

message ""
message "Check the edg_jobid parameter in the registered jdl"
run_command "$COMMAND --jdl $JOBID | grep edg_jobid | awk -F= '{print \$2}'| sed -e 's/\"//g' | sed -e 's/;//'" 0
if [[ $? -eq 0 ]] ; then
	JID=$(trim "$OUTPUT")
	if [[ $JID != $JOBID ]] ; then
		fail=$(($fail+1))
		message "Test fails. The registered jdl has not the correct JOBID: $JID"
	else
		message "Test success"
	fi
else
	fail=$(($fail+1))
	message "Command fails"
fi

message ""
message "Check the expiration time of the delegation"

run_command "$COMMAND -c $CONFIG_FILE $DELEGATION_OPTIONS | grep -m 1 Expiration | awk -F' : ' '{print \$2}'" 0 
if [[ $? -eq 0 ]] ; then
	dexp="$OUTPUT"
	run_command "$COMMAND -p $JOBID | grep -m 1 Expiration | awk -F' : ' '{print \$2}'" 0
	if [[ $? -eq 0 ]] ; then
		pexp="$OUTPUT"
		if [[ $dexp != $pexp ]] ; then
			fail=$(($fail+1))
			message "Test fails. The delegation proxy expires $dexp instead the user proxy expires $pexp"
		else
			message "Test success"
		fi
	else
		fail=$(($fail+1))
		message "Command fails"
	fi
else
	fail=$(($fail+1))
	message "Command fails"
fi


# ... terminate
if [ $fail -eq 1 ] ; then
  exit_failure "$fail test(s) fail(s)"
else
  exit_success
fi

