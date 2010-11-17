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
sleep 5
verbose "Check if the status is correct"
grep "\"Warning - JobPurging not allowed\"" $TMPFILE
if [[ $? -eq 1 ]] ; then
  get_STATUS $1
  if [[ "$JOBSTATUS" != *Cleared* ]]; then
    message "WARNING Job $1 not cleared!"
    return 1
  fi
else
  verbose "Warning: WMS is not recognized by the LB, JobPurging not allowed!"
fi 
}

function checkoutput() 
{
verbose "Check if the output files are correctly retrieved"
run_command "grep -A 1 'have been successfully retrieved' $TMPFILE | grep -v 'have been successfully retrieved'"
dir=$OUTPUT
verbose "Look if in $dir there are the output files std.out and std.err"
run_command ls ${dir}/std.out
run_command ls ${dir}/std.err
remove ${dir}/std.err
remove ${dir}/std.out
rmdir ${dir}
}

function check()
{
echo "$OUTPUT" > $TMPFILE
checkstatus $1
if [[ $? -eq 0 ]] ; then
  checkoutput
fi
}

prepare "test glite-wms-job-output command" $@

COMMAND=glite-wms-job-output
message "Check if command $COMMAND exists"
run_command which $COMMAND

# Test --version option
message ""
message "Test --version option"
run_command $COMMAND --version
verbose "Check the output command"
run_command grep "\"WMS User Interface version\"" <<< "$OUTPUT"
message "We are testing $OUTPUT"

# submit some jobs
run_command glite-wms-job-submit $DELEGATION_OPTIONS  --config $CONFIG_FILE --nomsg $JDLFILE
JOBID1=$OUTPUT
run_command glite-wms-job-submit $DELEGATION_OPTIONS  --config $CONFIG_FILE --nomsg $JDLFILE
JOBID2=$OUTPUT
run_command glite-wms-job-submit $DELEGATION_OPTIONS  --config $CONFIG_FILE --nomsg $JDLFILE
JOBID3=$OUTPUT
run_command glite-wms-job-submit $DELEGATION_OPTIONS  --config $CONFIG_FILE --nomsg $JDLFILE
JOBID4=$OUTPUT
run_command glite-wms-job-submit $DELEGATION_OPTIONS  --config $CONFIG_FILE --nomsg $JDLFILE
JOBID5=$OUTPUT
run_command glite-wms-job-submit $DELEGATION_OPTIONS  --config $CONFIG_FILE --nomsg $JDLFILE
JOBID6=$OUTPUT


wait_until_job_finishes $JOBID1
# Test option --logfile
message ""
message "Test --logfile option"
run_command $COMMAND --logfile $LOGFILE $JOBID1
check $JOBID1
verbose "Check the logfile"
run_command cat $LOGFILE
message ""
message "Try to purge a Cleared job"
run_command_fail $COMMAND --noint $JOBID1

wait_until_job_finishes $JOBID2
message ""
message "Test --dir option"
run_command $COMMAND --dir ${JOB_OUTPUT_DIR} $JOBID2
check $JOBID2

wait_until_job_finishes $JOBID3
message ""
message "Test --nosubdir option"
run_command $COMMAND --noint --nosubdir --dir ${JOB_OUTPUT_DIR} $JOBID3
check $JOBID3

wait_until_job_finishes $JOBID4
message ""
message "Test --list-only option"
run_command $COMMAND --list-only $JOBID4
echo "$OUTPUT" > $TMPFILE
run_command grep std.out $TMPFILE
run_command grep std.err $TMPFILE

message ""
message "Test --nopurge option"
run_command $COMMAND --nopurge $JOBID4
get_STATUS $JOBID4
if [[ "$JOBSTATUS" != *Cleared* ]]; then
  verbose "Try again to retrieve the output"
  run_command $COMMAND --noint --nopurge $JOBID4
else
  exit_failure "Job $JOBID4 has been purged"
fi

wait_until_job_finishes $JOBID5
wait_until_job_finishes $JOBID6
message ""
message "Test --input option"
echo $JOBID4 > $JOBIDFILE
echo $JOBID5 >> $JOBIDFILE
echo $JOBID6 >> $JOBIDFILE
run_command $COMMAND --noint --input $JOBIDFILE

exit_success
