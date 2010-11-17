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

START=$(date +%H:%M)
COMMAND=glite-wms-job-logging-info
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
run_command glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg --output $JOBIDFILE $JDLFILE
JOBID=$OUTPUT

message ""
message "Test a simple logging info..."
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

verbose "Submit a job"
run_command glite-wms-job-submit $DELEGATION_OPTIONS --output $JOBIDFILE --config $CONFIG_FILE $JDLFILE
# with file with more than 2 jobs it doesn't work for a bug
run_command glite-wms-job-submit $DELEGATION_OPTIONS --output $JOBIDFILE $JDLFILE

message ""
message "Test the --input option"
run_command $COMMAND --noint --config $CONFIG_FILE --input $JOBIDFILE 

sleep 60
NOW=$(date +%H:%M)        # format date, preserve leading zeros (e.g. 07:07)

message ""
message "Test the --to option"
run_command $COMMAND --config $CONFIG_FILE --to $NOW $JOBID	# this test fails on gLite UI 3.2.2

message ""
message "Test the --from option"
run_command $COMMAND --config $CONFIG_FILE --from $START $JOBID

message ""
message "Test the --event option (show only ACCEPTED events)"
run_command "$COMMAND --config $CONFIG_FILE --event ACCEPTED $JOBID | grep \"Event: Accepted\""

message ""
message "Test the --exclude option (exclude ACCEPTED events)"
run_command_fail "$COMMAND --config $CONFIG_FILE --exclude ACCEPTED $JOBID | grep \"Event: Accepted\""

message ""
message "Test all the options together (extract only EnQueued events)"
run_command $COMMAND --noint --input $JOBIDFILE --output $OUTPUTFILE --logfile $LOGFILE --config $CONFIG_FILE --verbosity 3 --from $START --to $NOW --event EnQueued
verbose "Check the log file" 
run_command cat $LOGFILE
verbose "Check the output file"
run_command cat $OUTPUTFILE

# ... terminate

exit_success
