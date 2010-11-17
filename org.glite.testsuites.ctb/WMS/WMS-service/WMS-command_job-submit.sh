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
# Test --start option
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

COMMAND=glite-wms-job-submit
message "Check if command $COMMAND exists"
run_command which $COMMAND

# Test --version option
message ""
message "Test --version option"
run_command $COMMAND --version
verbose "Check the output command"
run_command grep "\"WMS User Interface version\"" <<< "$OUTPUT"
verbose "We are testing $OUTPUT"

# Test option --endpoint
message ""
message "Test --endpoint option"
run_command $COMMAND $DELEGATION_OPTIONS --endpoint https://${WMS}:7443/glite_wms_wmproxy_server $JDLFILE

# Test option --output
message ""
message "Test --output option"
run_command $COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --output $OUTPUTFILE $JDLFILE
verbose "Check the output file"
run_command cat $OUTPUTFILE

# Test option --logfile
message ""
message "Test --logfile option"
run_command $COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --logfile $LOGFILE $JDLFILE
verbose "Check the logfile"
run_command cat $LOGFILE

# Test option --nomsg
message ""
message "Test --nomsg option"
run_command $COMMAND $DELEGATION_OPTIONS --nomsg --config $CONFIG_FILE $JDLFILE
debug "Job Id is $OUTPUT"
verbose "Check the status"
run_command glite-wms-job-status $OUTPUT

# Test all options together
message ""
message "Test all options together"
run_command $COMMAND $DELEGATION_OPTIONS --noint --logfile $LOGFILE --output $OUTPUTFILE --endpoint https://${WMS}:7443/glite_wms_wmproxy_server $JDLFILE
verbose "Check the output file"
run_command cat $OUTPUTFILE
remove $OUTPUTFILE
verbose "Check the logfile"
run_command cat $LOGFILE
remove $LOGFILE
 
# Test --input option
message ""
message "Test --input option"
verbose "Build CE file"
run_command glite-wms-job-list-match --config $CONFIG_FILE $DELEGATION_OPTIONS --rank --output $OUTPUTFILE $JDLFILE
grep "\"No Computing Element\"" $OUTPUTFILE
if [ $? == 0 ] ; then 
  awk -F ' ' '/:[[:digit:]]*\// {print $2}' $OUTPUTFILE > $TMPFILE
  run_command $COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --noint --input $TMPFILE --nomsg $JDLFILE
  JOBID=$OUTPUT
  wait_until_job_finishes $JOBID
  get_CE $JOBID
  verbose "Check if it has used the right ce"
  CE=`head -1 $TMPFILE`
  if [ $CE != $CENAME ] ; then
    exit_failure "Job has been submitted to the wrong ce: $CENAME (instead of $CE)"
  else
    verbose "Check success"
  fi
else
  verbose "No matching found. Test skipped."
fi
remove $TMPFILE
remove $OUTPUTFILE 


# Test --resource option
message ""
message "Test --resource option"
verbose "Look for a usable CE"
run_command glite-wms-job-list-match --noint --config $CONFIG_FILE $DELEGATION_OPTIONS --rank --output $OUTPUTFILE $JDLFILE
grep "\"No Computing Element\"" $OUTPUTFILE
if [ $? == 0 ] ; then 
  CE_ID=$(awk -F ' ' '/:[[:digit:]]*\// {print $2}' $OUTPUTFILE | head -1)
  run_command $COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --noint --resource $CE_ID --nomsg $JDLFILE
  JOBID=$OUTPUT
  wait_until_job_finishes $JOBID
  get_CE $JOBID
  verbose "Check if it has used the right ce"
  if [ $CE_ID != $CENAME ] ; then
    exit_failure "Job has been submitted to the wrong ce: $CENAME (instead of $CE_ID)"
  else
    verbose "Check success"
  fi
else
  verbose "No matching found. Test skipped."
fi
remove $OUTPUTFILE 

# Test option --register-only
message ""
message "Test --register-only option"
run_command $COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --register-only --output $JOBIDFILE $JDLFILE
JOBID=`tail -1 $JOBIDFILE`
verbose "Check if the output of the command is as expected"
run_command grep "-- \"--start $JOBID\"" <<< "$OUTPUT"
sleep 10 # wait some seconds...
get_STATUS $JOBID
if [[ "$JOBSTATUS" != *Submitted* ]]; then
  message "WARNING: The job $JOBID is not in the correct status. Its status is $JOBSTATUS."
else
  verbose "Check success"
fi
message ""
message "Test --start option"
run_command $COMMAND --start $JOBID
# ... wait loop with job-status calls
wait_until_job_finishes $JOBID
get_STATUS $JOBID
if [[ "$JOBSTATUS" != *Done* ]]; then
  message "WARNING: The job $JOBID fails. Its final status is $JOBSTATUS."
else
  verbose "Check success"
fi

# Test option --transfer-files
message ""
message "Test --transfer-files --proto option"
set_isbjdl $JDLFILE
run_command $COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --register-only --transfer-files --proto gsiftp --nomsg $JDLFILE
JOBID=$OUTPUT
get_STATUS $JOBID
if [[ "$JOBSTATUS" != *Submitted* ]]; then
  message "WARNING: The job $JOBID is not in the correct status. Its status is $JOBSTATUS."
else
  verbose "Check success"
fi
message ""
message "Test --start option"
run_command $COMMAND --start $JOBID
# ... wait loop with job-status calls
wait_until_job_finishes $JOBID

verbose "Retrieve the output"
run_command glite-wms-job-output --noint --nosubdir --dir $JOB_OUTPUT_DIR $JOBID
verbose "Check the output file"
if [ -f ${JOB_OUTPUT_DIR}/std.out ] && [ -f ${JOB_OUTPUT_DIR}/std.err ] ; then
  run_command grep "\"example.jdl\"" ${JOB_OUTPUT_DIR}/std.out
else
  exit_failure "Job output is not correct"
fi

message ""
message "Test --valid option"
NOW=$(date +%s)
((MYEXPIRY=10#$NOW+60)) # we ask 60seconds of validity
# ... submit a jdl valid for max 1 minute from NOW
set_requirements "\"false\"" # we need a jdl which doesn't match
run_command $COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg --valid 00:01 $JDLFILE
JOBID=$OUTPUT
sleep 30 # wait for job submission
# Check the status
get_STATUS $JOBID
if [[ "$JOBSTATUS" == *Waiting* ]]; then
  run_command grep "\"BrokerHelper: no compatible resources\"" <<< "$OUTPUT"
  verbose "Job doesn't match as expected"
else
  exit_failure "Job is in a wrong state: $JOBSTATUS"
fi
# Check the jdl
verbose "Check the job's jdl"
run_command glite-wms-job-info --noint -j --output $TMPFILE $JOBID
JDLEXPIRY=`grep ExpiryTime $TMPFILE | awk -F= '{print $2}' | sed -e "s/;//"`
## MYEXPIRY and JDLEXPIRY should be equal (or differ for 1 second) but there is a bug..
if [[ $MYEXPIRY < $(trim $JDLEXPIRY) ]] ; then
   exit_failure "Expiry time has not be set correctly! (*$MYEXPIRY* != *$JDLEXPIRY*)" 
else
   verbose "Attribute ExpiryTime has been correctly set in jdl"
fi
sleep 30 # wait until expiration
verbose "Wait until job aborts... this will take 10 minutes.."
wait_until_job_finishes $JOBID
get_STATUS $JOBID
if [[ "$JOBSTATUS" == *Aborted* ]]; then
  run_command grep "\"request expired\"" <<< "$OUTPUT"
  verbose "Job correctly aborts"
else
  exit_failure "Job is in a wrong state: $JOBSTATUS"
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
run_command $COMMAND $DELEGATION_OPTIONS --nomsg --config $CONFIG_FILE  --to $NOW $JDLFILE
JOBID=$OUTPUT
sleep 30 # wait for job submission
get_STATUS $JOBID
if [[ "$JOBSTATUS" == *Waiting* ]]; then
  run_command grep "\"BrokerHelper: no compatible resources\"" <<< "$OUTPUT"
else
  exit_failure "Job is in a wrong state: $JOBSTATUS"
fi
verbose "Check the job's jdl"
run_command glite-wms-job-info --noint -j --output $TMPFILE $JOBID
JDLEXPIRY=`grep ExpiryTime $TMPFILE | awk -F= '{print $2}' | sed -e "s/;//"`
MYEXPIRY=$(date +%s --date ${NOW})
if [[ $MYEXPIRY != $(trim $JDLEXPIRY) ]] ; then
   exit_failure "Expiry time has not be set correctly! (*$MYEXPIRY* != *$JDLEXPIRY*)" 
else
   verbose "Attribute ExpiryTime has been correctly set in jdl"
fi
sleep 30 # wait until expiration
verbose "Wait until job aborts... this will take 10 minutes.."
wait_until_job_finishes $JOBID
get_STATUS $JOBID
if [[ "$JOBSTATUS" == *Aborted* ]]; then
  run_command grep "\"request expired\"" <<< "$OUTPUT"
  verbose "Job correctly aborts"
else
  exit_failure "Job is in a wrong state: $JOBSTATUS"
fi

message ""
message "Test --default-jdl option"
# ... make a test default JDL file
echo "Attribute = 'Test default jdl';" >  $TMPFILE
run_command $COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg --default-jdl $TMPFILE $JDLFILE
JOBID=$OUTPUT
debug "Job ID is $JOBID"
verbose "Check the jdl"
run_command glite-wms-job-info -j $JOBID
run_command grep "\"Attribute = 'Test default jdl';\"" <<< "$OUTPUT"

# ... terminate

exit_success
