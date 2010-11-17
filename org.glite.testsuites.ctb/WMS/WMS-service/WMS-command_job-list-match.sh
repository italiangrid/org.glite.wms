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

COMMAND=glite-wms-job-list-match
message "Check if command $COMMAND exists"
run_command which $COMMAND

# Test --version option
message ""
message "Test --version option"
run_command $COMMAND --version
verbose "Check the output command"
run_command grep "\"WMS User Interface version\"" <<< "$OUTPUT"
message "We are testing $OUTPUT"

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

# use option --rank
message ""
message "Test --rank option"
run_command $COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --rank $JDLFILE

## try a failure matching
message ""
message "Try a failure matching (Requirements == false)"
set_requirements "\"false\""
run_command $COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE  $JDLFILE
verbose "Check output"
run_command grep "\"No Computing Element matching your job requirements has been found!\"" <<< "${OUTPUT}"

## try a restricted requirements
message ""
message "Match only Cream's CE"
set_requirements "regexp(\"8443/cream-\", other.GlueCEUniqueID)"
run_command $COMMAND $DELEGATION_OPTIONS --debug --config $CONFIG_FILE $JDLFILE
verbose "Look for LCG-CE in the command's output"
run_command_fail grep "\"2118/jobmanager-\"" <<< "${OUTPUT}"

## use a previouly delegated proxy
message ""
message "Use a previouly delegated proxy"
define_delegation
set_jdl $JDLFILE
run_command $COMMAND $DELEGATION_OPTIONS --config $CONFIG_FILE --rank $JDLFILE

# ... terminate
exit_success
