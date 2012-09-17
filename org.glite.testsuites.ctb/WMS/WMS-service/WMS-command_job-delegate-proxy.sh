#!/bin/sh

###############################################################################
#
# A glite-wms-job-delegate-proxy command test
#
# This test is intended to verify glite-wms-job-delegate-proxy command
# Test these options: --version --logfile --output --endpoint --autm-delegation 
# -> success means that command has success, delegation is stored in the WMS
# -> and that we are able to submit a job
# 
# The following tests are possible only if test option -i is abilitate
# -> Delegate a shorter proxy 
# -> Try to delegate with an expired proxy
# -> Try to submit with an expired delegation
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare "test glite-wms-job-delegate-proxy command" $@

# check if delegation exists and try a submit
function checkdeleg()
{
	ret=0
  verbose "Verify the delegation"
  run_command "glite-wms-job-info -d $1 --config $CONFIG_FILE" 0
	if [[ $? -eq 1 ]] ; then message "Delegation doesn't exists"; ret=1; fi
  verbose "Try a submit"
  run_command "glite-wms-job-submit -d $1 --config $CONFIG_FILE --nomsg $JDLFILE" 0
	if [[ $? -eq 1 ]] ; then message "Submission fail"; ret=1; fi
  JOBID=$OUTPUT
  verbose "Cancel the unused job"
 	run_command "glite-wms-job-cancel --noint $JOBID" 0
	if [[ $? -eq 1 ]] ; then verbose "Warning: cannot cancel $JOBID"; fi
	return $ret
}


fail=0

COMMAND=glite-wms-job-delegate-proxy
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
    verbose "Warning: version number not found ($OUTPUT)"
  fi
else
  fail=$(($fail+1))
  message "Command fails"
fi

# test option --autm-delegation
message ""
message "Test option --autm-delegation"
run_command "$COMMAND --config $CONFIG_FILE --output $OUTPUTFILE --autm-delegation" 0
if [[ $? -eq 0 ]] ; then
	run_command "awk -F ' ' '/delegation/ {print \$NF}' $OUTPUTFILE" 0
	if [[ $? -eq 0 ]] ; then
		DELEGATIONID="$OUTPUT"
		verbose "Delegation Id: $DELEGATIONID"
		checkdeleg $DELEGATIONID
		if [[ $? -eq 0 ]] ; then
			message "Test success"
		else
			fail=$(($fail+1))
			message "Test fails. Wrong delegation ($DELEGATIONID)"
		fi
	else
		fail=$(($fail+1))
		message "Cannot obtain delegation"
	fi
else
	fail=$(($fail+1))
	message "Command fails"
fi
remove $OUTPUTFILE

# use a predefined delegation name (option -d)
message ""
message "Test option -d"
DELEGATIONID="deleg-$$"
run_command "$COMMAND --config $CONFIG_FILE  -d $DELEGATIONID" 0
if [[ $? -eq 0 ]] ; then
	checkdeleg $DELEGATIONID
	if [[ $? -eq 0 ]] ; then
    message "Test success"
  else
    fail=$(($fail+1))
    message "Test fails. Wrong delegation ($DELEGATIONID)"
  fi
else
	fail=$(($fail+1))
  message "Command fails"
fi

# test option --logfile
message ""
message "Test option --logfile"
run_command "$COMMAND --config $CONFIG_FILE  --logfile $LOGFILE -d $DELEGATIONID" 0
if [[ $? -eq 0 ]] ; then
	verbose "Check logfile"
	run_command "cat $LOGFILE" 0
	if [[ $? -eq 0 ]] ; then
  	checkdeleg $DELEGATIONID
  	if [[ $? -eq 0 ]] ; then
    	message "Test success"
  	else
    	fail=$(($fail+1))
    	message "Test fails. Wrong delegation ($DELEGATIONID)"
  	fi
	else
		fail=$(($fail+1))
		message "Test fails. Logfile file not created"
	fi
else
  fail=$(($fail+1))
  message "Command fails"
fi
remove $LOGFILE

# test option --endpoint
message ""
message "Test option --endpoint"
run_command "$COMMAND --config $CONFIG_FILE  --endpoint  https://${WMS}:7443/glite_wms_wmproxy_server -d $DELEGATIONID --output $OUTPUTFILE" 0
if [[ $? -eq 0 ]] ; then
	run_command "awk -F ' ' '/WMProxy/ {print \$NF}' $OUTPUTFILE" 0
  if [[ $? -eq 0 ]] ; then
		ENDPOINT="$OUTPUT"
		if [[ $ENDPOINT != "https://${WMS}:7443/glite_wms_wmproxy_server" ]] ; then
			fail=$(($fail+1))
			message "Test fails. Wrong endpoint delegating proxy ($ENDPOINT)"
		else
    	checkdeleg $DELEGATIONID
    	if [[ $? -eq 0 ]] ; then
      	message "Test success"
    	else
      	fail=$(($fail+1))
      	message "Test fails. Wrong delegation ($DELEGATIONID)"
    	fi
		fi
  else
    fail=$(($fail+1))
    message "Cannot obtain endpoint"
  fi
else
  fail=$(($fail+1))
  message "Command fails"
fi
remove $OUTPUTFILE

# test all options
touch $OUTPUTFILE $LOGFILE # create fake files
message ""
message "Test all options together"
run_command "$COMMAND --noint --output $OUTPUTFILE --logfile $LOGFILE --config $CONFIG_FILE --endpoint $ENDPOINT -d $DELEGATIONID" 0
if [[ $? -eq 0 ]] ; then
  verbose "Check the output"
	run_command "cat $OUTPUTFILE" 0
  if [[ $? -eq 0 ]] ; then
    verbose "Check the logfile"
		run_command "cat $LOGFILE" 0
    if [[ $? -ne 0 ]] ; then
      fail=$(($fail+1))
      message "Test fails. Logfile file not created"
    else
      checkdeleg $DELEGATIONID
      if [[ $? -eq 0 ]] ; then
        message "Test success"
      else
        fail=$(($fail+1))
        message "Test fails. Wrong delegation ($DELEGATIONID)"
      fi
    fi
  else
    fail=$(($fail+1))
    message "Test fails. Output file not created"
  fi
else
  fail=$(($fail+1))
  message "Command fails"
fi
remove $LOGFILE
remove $OUTPUTFILE

# for the next tests we need user proxy password
if [ $NOPROXY -eq 0 ] ; then 
  # create a shorter delegation
  set_proxy "$PROXY" "00:10"
  message ""
  message "Try to delegate with a short proxy and check the validity"
  run_command "$COMMAND --config $CONFIG_FILE  -d $DELEGATIONID" 0
	if [[ $? -eq 0 ]] ; then
		verbose "Check the delegation timeleft value"
  	run_command "glite-wms-job-info -d $DELEGATIONID --config $CONFIG_FILE --output $OUTPUTFILE" 0
		if [[ $? -eq 0 ]] ; then
			run_command "grep -m 1 Timeleft $OUTPUTFILE | awk '{print \$4}'" 0
			if [[ $? -eq 0 ]] ; then
				text="$OUTPUT"
				run_command "grep -m 1 Timeleft $OUTPUTFILE | awk '{print \$3}'" 0
				if [[ $? -eq 0 ]] ; then
					value="$OUTPUT"
					remove $OUTPUTFILE
					if [ $text != "min" ] || [ $value -gt 10 ] ; then
						fail=$(($fail+1))
						message "Test fails. Wrong timeleft delegating proxy"
					else
						checkdeleg $DELEGATIONID
						if [[ $? -eq 0 ]] ; then
        			message "Test success"
      			else
        			fail=$(($fail+1))
       	 			message "Test fails. Wrong delegation ($DELEGATIONID)"
      			fi
					fi	
				else
					fail=$(($fail+1))
					message "Cannot retrieve timeleft value"
				fi
			else
				fail=$(($fail+1))
				message "Cannot retrieve timeleft value"
			fi
		else
			fail=$(($fail+1))
			message	"Cannot retrieve information about delegation ($DELEGATIONID)"
		fi
	else
		fail=$(($fail+1))
		message "Command fails"
  fi
		
  # try with an expiry delegation   
  set_proxy "$PROXY" "00:01"
  message ""
  message "Works with expiring proxy..." 
  run_command "$COMMAND --config $CONFIG_FILE  -d $DELEGATIONID" 0
	if [[ $? -eq 0 ]] ; then
		verbose "Wait until proxy expired..."
		run_command "sleep 60" 0
		# first try to delegate again with an expired proxy
		message "Try to delegate with an expired proxy"
		run_command_fail "$COMMAND --config $CONFIG_FILE --autm-delegation" 0
		if [[ $? -eq 0 ]] ; then
			# refresh the proxy
			set_proxy "$PROXY"
			# then check if the old delegation is expired	
			verbose "Check if the old delegation is expired"
			run_command_fail "glite-wms-job-info  --config $CONFIG_FILE -d $DELEGATIONID | grep Timeleft" 0
			if [[ $? -eq 0 ]] ; then
				# then try to submit with the expired delegation
				message "Try to submit with an expired delegation"
				run_command_fail "glite-wms-job-submit -d $DELEGATIONID --config $CONFIG_FILE --logfile $LOGFILE $JDLFILE" 0
				if [[ $? -eq 0 ]] ; then
					verbose "Check the output of the command"
					run_command "grep \"The delegated Proxy has expired\" $LOGFILE" 0
					if [[ $? -eq 0 ]] ; then
						message "Test success"
					else
						fail=$(($fail+1))
						message "Command not failed for the expected reason (expired proxy)"
					fi
				else
					fail=$(($fail+1))
					message "Command not failed as expected"
				fi
			else
				fail=$(($fail+1))
				message "Command not failed as expected"
			fi
		else
			fail=$(($fail+1))
			message "Command not failed as expected"
		fi
	else
    fail=$(($fail+1))
    message "Command fails"
  fi
  remove $LOGFILE
fi

# ... terminate
if [ $fail -ne 0 ] ; then
  exit_failure "$fail test(s) fail(s)"
else
  exit_success
fi

