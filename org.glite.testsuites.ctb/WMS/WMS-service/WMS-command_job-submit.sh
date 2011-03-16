#!/bin/sh

###############################################################################
#
# test glite-wms-job-submit command.
#
# Test --version option
# Test --endpoint option
# Test --output option
# Test --logfile option
# Test --nomsg option
# Test all options together
# Test --input option
# Test --resource option
# Test --register-only option
# Test --start option
# Test --transfer-files --proto option
# Test --valid option
# Test --to option
# Test --default-jdl option
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare "test glite-wms-job-submit command" $@

fail=0

COMMAND=glite-wms-job-submit
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

# Test option --endpoint
message ""
message "Test --endpoint option"
run_command "$COMMAND $DELEGATION_OPTIONS --endpoint https://${WMS}:7443/glite_wms_wmproxy_server $JDLFILE" 0
if [[ $? -eq 0 ]] ; then
  message "Test success"
else
  fail=$(($fail+1))
  message "Test fails."
fi

# Test option --output
message ""
message "Test --output option"
run_command "$COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --output $OUTPUTFILE $JDLFILE" 0
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

# Test option --nomsg
message ""
message "Test --nomsg option"
run_command "$COMMAND $DELEGATION_OPTIONS --nomsg --config $CONFIG_FILE $JDLFILE" 0
if [[ $? -eq 0 ]] ; then
	debug "Job Id is $OUTPUT"
	verbose "Check the status"
	run_command "glite-wms-job-status $OUTPUT" 0
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

# Test --input option
message ""
message "Test --input option"
verbose "Build CE file"
run_command "glite-wms-job-list-match --config $CONFIG_FILE $DELEGATION_OPTIONS --rank --output $OUTPUTFILE $JDLFILE" 0
if [[ $? -eq 0 ]] ; then
	run_command_fail "grep \"No Computing Element\" $OUTPUTFILE" 0
	if [[ $? -eq 0 ]] ; then 
  	run_command "awk -F ' ' '/:[[:digit:]]*\// {print \$2}' $OUTPUTFILE > $TMPFILE" 0
		if [[ $? -eq 0 ]] ; then
			run_command "$COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --noint --input $TMPFILE --nomsg $JDLFILE" 0
			if [[ $? -eq 0 ]] ; then
  			JOBID=$OUTPUT
  			wait_until_job_finishes $JOBID
  			get_CE $JOBID
  			verbose "Check if it has used the right ce"
  			run_command "head -1 $TMPFILE" 0
  			if [[ $? -ne 0 ]] || [[ $OUTPUT != $CENAME ]] ; then
					fail=$(($fail+1))
    			message "Test fails. Job has been submitted to the wrong ce: $CENAME (instead of $OUTPUT)"
 			 	else
    			message "Test success"
				fi
			else
				fail=$(($fail+1))
				message "Command fails"
			fi
		else
			fail=$(($fail+1))
			message "Test fails. Cannot create CE files"
		fi
	else
		message "No matching found. Test skipped."
  fi
else
  message "No matching found. Test skipped."
fi
remove $TMPFILE
remove $OUTPUTFILE 


# Test --resource option
message ""
message "Test --resource option"
verbose "Look for a usable CE"
run_command "glite-wms-job-list-match --noint --config $CONFIG_FILE $DELEGATION_OPTIONS --rank --output $OUTPUTFILE $JDLFILE" 0
if [[ $? -eq 0 ]] ; then
	run_command_fail "grep \"No Computing Element\" $OUTPUTFILE" 0
	if [[ $? -eq 0 ]] ; then 
  	run_command "awk -F ' ' '/:[[:digit:]]*\// {print \$2}' $OUTPUTFILE | head -1" 0
		if [[ $? -eq 0 ]] ; then
			CE_ID=$OUTPUT
  		run_command "$COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --noint --resource $CE_ID --nomsg $JDLFILE" 0
			if [[ $? -eq 0 ]] ; then
  			JOBID=$OUTPUT
  			wait_until_job_finishes $JOBID
  			get_CE $JOBID
  			verbose "Check if it has used the right ce"
  			if [ $CE_ID != $CENAME ] ; then
					fail=$(($fail+1))
    			message "Test fails. Job has been submitted to the wrong ce: $CENAME (instead of $CE_ID)"
  			else
    			message "Test success"
				fi
			else
				fail=$(($fail+1))
				message "Command fails"
			fi
		else
			fail=$(($fail+1))
			message "Test fails. Cannot found a suitable CE"
    fi
  else
    message "No matching found. Test skipped."
  fi
else
  message "No matching found. Test skipped."
fi
remove $OUTPUTFILE 

# Test option --register-only
message ""
message "Test --register-only option"
run_command "$COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --register-only --output $JOBIDFILE $JDLFILE" 0
if [[ $? -eq 0 ]] ; then
	JOBID=`tail -1 $JOBIDFILE`
	verbose "Check if the output of the command is as expected"
	run_command "grep -- \"--start $JOBID\" $CMDOUT" 0
	if [[ $? -eq 0 ]] ; then
		run_command "sleep 10" 0 # wait some seconds...
		get_STATUS $JOBID
		if [[ "$JOBSTATUS" != *Submitted* ]]; then
  		message "WARNING: The job $JOBID is not in the correct status. Its status is $JOBSTATUS."
		else
  		message "Test success"
		fi	
	else
		fail=$(($fail+1))
		message "Test fails"
	fi
else
	fail=$(($fail+1))
	message "Command fails"
fi

# Test option --transfer-files
message ""
message "Test --transfer-files --proto option"
set_isbjdl $JDLFILE #use a jdl with ISB
run_command "$COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --register-only --transfer-files --proto gsiftp --nomsg $JDLFILE" 0
if [[ $? -eq 0 ]] ; then
	sleep 10
	JOBID=$OUTPUT
	get_STATUS $JOBID
  if [[ "$JOBSTATUS" != *Submitted* ]]; then
    message "Warning The job $JOBID fails. Its final status is $JOBSTATUS."
  else
    message "Test success"
  fi
else
  fail=$(($fail+1))
  message "Command fails"
fi

message ""
message "Test --start option"
run_command "$COMMAND --start $JOBID" 0
if [[ $? -eq 0 ]] ; then
	wait_until_job_finishes $JOBID
	verbose "Retrieve the output"
	run_command "glite-wms-job-output --noint --nosubdir --dir $JOB_OUTPUT_DIR $JOBID" 0
	if [[ $? -eq 0 ]] ; then
		verbose "Check the output file"
		if [ -f ${JOB_OUTPUT_DIR}/std.out ] && [ -f ${JOB_OUTPUT_DIR}/std.err ] ; then
			run_command "grep \"example.jdl\" ${JOB_OUTPUT_DIR}/std.out" 0
			if [[ $? -eq 0 ]] ; then
				message "Test success"
			else
				fail=$(($fail+1))
				message "Test fails. Output files are not correct"
			fi
		else
			fail=$(($fail+1))
			message "Test fails. Cannot find output files"
		fi
	else
		fail=$(($fail+1))	
		message "Test fails. Cannot retrieve output files"
	fi
else
  fail=$(($fail+1))
  message "Command fails"
fi

message ""
message "Test --valid option"
NOW=$(date +%s)
((MYEXPIRY=10#$NOW+60)) # we ask 60seconds of validity
# ... submit a jdl valid for max 1 minute from NOW
set_requirements "\"false\"" # we need a jdl which doesn't match
run_command "$COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg --valid 00:01 $JDLFILE" 0
if [[ $? -eq 0 ]] ; then
	JOBID=$OUTPUT
	sleep 30 # wait for job submission
	# Check the status
	get_STATUS $JOBID
	if [[ "$JOBSTATUS" == *Waiting* ]]; then
  	run_command "glite-wms-job-status $JOBID | grep \"BrokerHelper: no compatible resources\"" 0
		if [[ $? -eq 0 ]] ; then
  		# Check the jdl
			verbose "Check the job's jdl"
			run_command "glite-wms-job-info --noint -j --output $TMPFILE $JOBID" 0
			if [[ $? -eq 0 ]] ; then
				run_command "grep ExpiryTime $TMPFILE | awk -F= '{print \$2}' | sed -e 's/;//'" 0
				if [[ $? -eq 0 ]] ; then
					JDLEXPIRY=$(trim "$OUTPUT")
					## MYEXPIRY and JDLEXPIRY should be equal (or differ for 1 second) but there is a bug..
					if [[ $MYEXPIRY -ge $JDLEXPIRY ]] ; then
						verbose "Attribute ExpiryTime has been correctly set in jdl"
						sleep 30  # wait until expiration
						verbose "Wait until job aborts... this will take 10 minutes.."
						wait_until_job_finishes $JOBID
						get_STATUS $JOBID
						if [[ "$JOBSTATUS" == *Aborted* ]]; then
							run_command "glite-wms-job-status $JOBID | grep \"request expired\"" 0
							if [[ $? -eq 0 ]] ; then
								message "Test success"
							else
								fail=$(($fail+1))
								message "Test fails. Job ($JOBID) fails with not expectetd reason"
							fi
						else
							fail=$(($fail+1))
							message "Test fails. Job ($JOBID) is in a wrong state: $JOBSTATUS"
						fi
					else
						fail=$(($fail+1))
						message "Test fails. Expiry time has not be set correctly! (*$MYEXPIRY* != *$JDLEXPIRY*)"	
					fi
				else
					fail=$(($fail+1))
					message "Test fails. Cannot extract ExpiryTime value"
				fi
			else
				fail=$(($fail+1))
				message "Test fails. Cannot obtain information for $JOBID"
			fi
		else
			fail=$(($fail+1))
			message "Test fails. Job ($JOBID) unexpectly matches"
		fi
	else
		fail=$(($fail+1))
		message "Test fails. Job ($JOBID) is in a wrong state: $JOBSTATUS"
	fi
else
  fail=$(($fail+1))
  message "Command fails"
fi

message ""
message "Test --to option"
# ... make a timestamp which is 1 min (max) from now
NOWHOUR=$(date +%H)
NOWMIN=$(date +%M)
((NOWMIN=10#$NOWMIN+1))                                 # increment (force decimal system to be used)
if [[ $NOWMIN -eq 60 ]]; then
  NOWMIN=0
  if [[ $NOWHOUR -eq 23 ]]; then
    NOWHOUR=0
  else 
    ((NOWHOUR=10#$NOWHOUR+1)) 
  fi
fi
NOW=$(date +%H:%M --date ${NOWHOUR}:${NOWMIN})        # format date, preserve leading zeros (e.g. 07:07)
# ... submit a jdl valid for max 1 minute from NOW
set_requirements "\"false\"" # we need a jdl which doesn't match
run_command "$COMMAND $DELEGATION_OPTIONS --nomsg --config $CONFIG_FILE  --to $NOW $JDLFILE" 0
if [[ $? -eq 0 ]] ; then
	JOBID=$OUTPUT
	sleep 10 # wait for job submission
	get_STATUS $JOBID
	if [[ "$JOBSTATUS" == *Waiting* ]]; then
  	run_command "glite-wms-job-status $JOBID | grep \"BrokerHelper: no compatible resources\"" 0
		if [[ $? -eq 0 ]] ; then
			verbose "Check the job's jdl"
			run_command "glite-wms-job-info --noint -j --output $TMPFILE $JOBID" 0
			if [[ $? -eq 0 ]] ; then
				run_command "grep ExpiryTime $TMPFILE | awk -F= '{print \$2}' | sed -e 's/;//'" 0
				if [[ $? -eq 0 ]] ; then
					JDLEXPIRY=$(trim "$OUTPUT")
					MYEXPIRY=$(date +%s --date ${NOW})
					if [[ $MYEXPIRY == $JDLEXPIRY ]] ; then
						run_command "sleep 30" 0 # wait until expiration
						verbose "Wait until job aborts... this will take 10 minutes.."
						wait_until_job_finishes $JOBID
            get_STATUS $JOBID
            if [[ "$JOBSTATUS" == *Aborted* ]]; then
              run_command "glite-wms-job-status $JOBID | grep \"request expired\"" 0
              if [[ $? -eq 0 ]] ; then
                message "Test success"
              else
                fail=$(($fail+1))
                message "Test fails. Job ($JOBID) fails with not expectetd reason"
              fi
            else
              fail=$(($fail+1))
              message "Test fails. Job ($JOBID) is in a wrong state: $JOBSTATUS"
            fi
          else
            fail=$(($fail+1))
            message "Test fails. Expiry time has not be set correctly! (*$MYEXPIRY* != *$JDLEXPIRY*)"
          fi
        else
          fail=$(($fail+1))
          message "Test fails. Cannot extract ExpiryTime value"
        fi
      else
        fail=$(($fail+1))
        message "Test fails. Cannot obtain information for $JOBID"
      fi
    else
      fail=$(($fail+1))
      message "Test fails. Job ($JOBID) unexpectly matches"
    fi
  else
    fail=$(($fail+1))
    message "Test fails. Job ($JOBID) is in a wrong state: $JOBSTATUS"
  fi
else
  fail=$(($fail+1))
  message "Command fails"
fi
	
			
message ""
message "Test --default-jdl option"
# ... make a test default JDL file
echo "Attribute = 'Test default jdl';" >  $TMPFILE
run_command "$COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg --default-jdl $TMPFILE $JDLFILE" 0
if [[ $? -eq 0 ]] ; then
	JOBID=$OUTPUT
	debug "Job ID is $JOBID"
	verbose "Check the jdl"
	run_command "glite-wms-job-info -j $JOBID" 0
	if [[ $? -eq 0 ]] ; then
		run_command "grep \"Attribute = 'Test default jdl';\" $CMDOUT" 0
		if [[ $? -eq 0 ]] ; then
			message "Test success"
		else
			fail=$(($fail+1))
			message "Test fails."
		fi
	else
		fail=$(($fail+1))
		message "Test fails. Cannot obtain information for $JOBID"
	fi
else
  fail=$(($fail+1))
  message "Command fails"
fi

# ... terminate

if [ $fail -ne 0 ] ; then
  exit_failure "$fail test(s) fail(s)"
else 
  exit_success
fi  

