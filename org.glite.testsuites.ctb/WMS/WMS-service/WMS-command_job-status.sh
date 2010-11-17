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

START=$(date +%H:%M)
COMMAND=glite-wms-job-status
message "Check if command $COMMAND exists"
run_command which $COMMAND

# Test --version option
message ""
message "Test --version option"
run_command $COMMAND --version
verbose "Check the output command"
run_command grep "\"User Interface version\"" <<< "$OUTPUT"
message "We are testing $OUTPUT"

verbose "Submit a job"
run_command glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg $JDLFILE
JOBID=$OUTPUT

message ""
message "Test a simple status..."
run_command $COMMAND $JOBID

message ""
message "Test option --config"
run_command $COMMAND --config $CONFIG_FILE $JOBID

message ""
message "Test option --output"
run_command $COMMAND --output $OUTPUTFILE --config $CONFIG_FILE $JOBID
verbose "Check the output file"
run_command cat $OUTPUTFILE
rm -f $OUTPUTFILE

message ""
message "Test option --logfile"
run_command $COMMAND --logfile $LOGFILE --config $CONFIG_FILE $JOBID
verbose "Check the log file" 
run_command cat $LOGFILE
rm -f $LOGFILE

verbose "Submit some jobs"
run_command glite-wms-job-submit $DELEGATION_OPTIONS --output $JOBIDFILE --config $CONFIG_FILE $JDLFILE
run_command glite-wms-job-submit $DELEGATION_OPTIONS --output $JOBIDFILE --config $CONFIG_FILE $JDLFILE
run_command glite-wms-job-submit $DELEGATION_OPTIONS --output $JOBIDFILE --config $CONFIG_FILE $JDLFILE

message ""
message "Test the --input option"
run_command $COMMAND --noint --config $CONFIG_FILE --input $JOBIDFILE 

verbose "Check if LB has indexing capabilities"
out=$($COMMAND --config $CONFIG_FILE --noint --all 2>&1)
if [ $? -eq 0 ] ; then

  set_requirements "\"false\""
  echo "usertags = [ type = \"test job\" ];" >> $JDLFILE
  run_command glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --output $JOBIDFILE $JDLFILE
  JOBID=`tail -1 $JOBIDFILE`

  verbose "Waiting until job arrives to the expected status.."
  get_STATUS $JOBID
  while [[ "$JOBSTATUS" != *Waiting* ]] ; do
    get_STATUS $JOBID
    sleep 5
  done

  message ""
  message "Test the --exclude option (exclude Waiting,  Done, Cleared and Aborted jobs)"
  run_command $COMMAND --config $CONFIG_FILE --all --exclude Waiting --exclude Done --exclude 8 --exclude 7
  run_command_fail grep $JOBID <<< "$OUTPUT"

  message ""
  message "Test the --status option (look for Waiting job)"
  run_command $COMMAND --config $CONFIG_FILE --all --status Waiting
  run_command grep $JOBID <<< "$OUTPUT"

  message ""
  message "Test the --user-tag option"
  run_command $COMMAND --config $CONFIG_FILE --all --user-tag type=\"test job\" 
  run_command grep $JOBID <<< "$OUTPUT"
  run_command_fail $COMMAND --config $CONFIG_FILE -all --user-tag type=\"wrong tag\" 

else
  message ""
  message "BEWARE: Test other options requires certain indexing capabilities to be enabled on the LB server!"
fi


# we need to test --verbosity --to --from
#
# ... terminate

exit_success
