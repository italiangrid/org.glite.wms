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
  run_command glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg --output $JOBIDFILE $JDLFILE
  JOBID=$OUTPUT
  sleep 5 # wait until job arrives to wm, needs because there is a bug in the wmproxy.
}

function check_status()
{
  i=0
  get_STATUS $JOBID

  # try to check the status 5 times (for a total of 75 seconds) 
  while [[ $JOBSTATUS != "Cancelled" ]] &&  [[ $i < 5 ]] ; do 
    ((i=$i+1))
    sleep $(($i*5))
    get_STATUS $JOBID
  done

  # if all tries fail exit with failure 
  if [[ $JOBSTATUS != "Cancelled" ]] ; then
    if is_finished $JOBID ; then
      exit_failure "Job status is wrong: $JOBSTATUS"
    else
      message "WARNING: job finished ($JOBSTATUS) before cancellation..."
    fi
  fi
}

# ... startup

. $(dirname $0)/functions.sh

prepare "test command glite-wms-job-cancel" $@

COMMAND=glite-wms-job-cancel
message "Check if command $COMMAND exists"
run_command which $COMMAND

# Test --version option
message ""
message "Test --version option"
run_command $COMMAND --version
verbose "Check the output command"
run_command grep "\"WMS User Interface version\"" <<< "$OUTPUT"
message "We are testing $OUTPUT"

set_longjdl $JDLFILE # use long job

# ... submit a job - cancel job. with different options

message ""
message "Test --config option"
submit_job
is_finished $JOBID
if [[ $? -eq 0 ]] ; then # Cancel the job only if it is not finished
  run_command $COMMAND --config $CONFIG_FILE --noint $JOBID
fi
check_status
remove $JOBIDFILE # remove unused jobid file

message ""
message "Test --output option"
submit_job
is_finished $JOBID
if [[ $? -eq 0 ]]; then
  run_command $COMMAND --config $CONFIG_FILE --noint --output $OUTPUTFILE $JOBID
fi
check_status
remove $JOBIDFILE
verbose "Check output file:"
run_command cat $OUTPUTFILE

message ""
message "Test --logfile option"
submit_job
is_finished $JOBID
if [[ $? -eq 0 ]] ; then
  run_command $COMMAND --config $CONFIG_FILE --noint --logfile $LOGFILE $JOBID
fi
check_status
remove $JOBIDFILE
verbose "Check logfile:"
run_command cat $LOGFILE

message ""
message "Test --input option"
# to test "input" option submit 5 jobs
i=0
while [[ $i < 5 ]] ; do
  submit_job
  ((i=$i+1))
done
run_command $COMMAND --config $CONFIG_FILE --noint --input $JOBIDFILE
for JOBID in `cat $JOBIDFILE`; do
  if [[ "$JOBID" == "http"* ]]; then 
    verbose "Check status of $JOBID"
    check_status
  fi
done
remove $JOBIDFILE  

message ""
message "Test all options together"
submit_job
is_finished $JOBID
if [[ $? -eq 0 ]] ; then
  run_command $COMMAND --noint --input $JOBIDFILE --output $OUTPUTFILE --logfile $LOGFILE --config $CONFIG_FILE
fi
check_status
verbose "Check logfile:"
run_command cat $LOGFILE
verbose "Check outfile:"
run_command cat $OUTPUTFILE
remove $JOBIDFILE

set_jdl $JDLFILE # use a short job

message ""
message "Try to cancel a DoneOk job"
submit_job
wait_until_job_finishes $JOBID
is_finished $JOBID
if [[ $? -ne 0 ]] ; then
  run_command_fail $COMMAND --noint $JOBID
fi

# ... terminate

exit_success
