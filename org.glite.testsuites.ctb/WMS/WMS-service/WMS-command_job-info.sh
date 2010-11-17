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

COMMAND=glite-wms-job-info
message "Check if command $COMMAND exists"
run_command which $COMMAND

# Test --version option
message ""
message "Test --version option"
run_command $COMMAND --version
verbose "Check the output command"
run_command grep "\"WMS User Interface version\"" <<< "$OUTPUT"
message "We are testing $OUTPUT"

# ... submit a job

define_delegation
verbose "Submit a job"
run_command glite-wms-job-submit $DELEGATION_OPTIONS -c $CONFIG_FILE --nomsg --output $JOBIDFILE $JDLFILE
JOBID=$OUTPUT

# test these manadatory options --jdl-original --jdl --proxy 
# combined with differents extra options

for opt in --jdl-original --jdl --proxy ; do

  message ""
  message "Test $COMMAND $opt with --input option"
  run_command $COMMAND --input $JOBIDFILE $opt

  message ""
  message "Test $COMMAND $opt with --config option"
  run_command $COMMAND --config $CONFIG_FILE $opt $JOBID

  message ""
  message "Test $COMMAND $opt with --output option"
  run_command $COMMAND --output $OUTPUTFILE $opt $JOBID
  verbose "Check the output"
  run_command cat $OUTPUTFILE

  message ""
  message "Test $COMMAND $opt with --logfile option"
  run_command $COMMAND --logfile $LOGFILE $opt $JOBID
  verbose "Check the logfile"
  run_command cat $LOGFILE

  message ""
  message "Test $COMMAND $opt with --endpoint option"
  run_command $COMMAND --endpoint https://${WMS}:7443/glite_wms_wmproxy_server $opt $JOBID

  message ""
  message "Test $COMMAND $opt with all options"
  run_command $COMMAND --noint --debug --input $JOBIDFILE --config $CONFIG_FILE  --output $OUTPUTFILE  --logfile $LOGFILE $opt
  verbose "Check the output file"
  run_command cat $OUTPUTFILE
  verbose "Check the logfile"
  run_command cat $LOGFILE
  remove $LOGFILE
  remove $OUTPUTFILE

done

## test --delegationid mandatory option"

message ""
message "Test $COMMAND --delegationid with --config option"
run_command $COMMAND --config $CONFIG_FILE $DELEGATION_OPTIONS

message ""
message "Test $COMMAND --delegationid with --output option"
run_command $COMMAND --output $OUTPUTFILE --config $CONFIG_FILE $DELEGATION_OPTIONS
verbose "Check the output file"
run_command cat $OUTPUTFILE

message ""
message "Test $COMMAND --delegationid with --logfile option"
run_command $COMMAND --logfile $LOGFILE --config $CONFIG_FILE $DELEGATION_OPTIONS
verbose "Check the logfile"
run_command cat $LOGFILE

message ""
message "Test $COMMAND --delegationid with with --endpoint option"
run_command $COMMAND --endpoint https://${WMS}:7443/glite_wms_wmproxy_server $DELEGATION_OPTIONS

message ""
message "Test $COMMAND --delegationid with all options"
run_command $COMMAND --noint --debug --config $CONFIG_FILE --output $OUTPUTFILE --logfile $LOGFILE --endpoint https://${WMS}:7443/glite_wms_wmproxy_server $DELEGATION_OPTIONS
verbose "Check the output file"
run_command cat $OUTPUTFILE
verbose "Check the logfile"
run_command cat $LOGFILE
remove $LOGFILE
remove $OUTPUTFILE


## some ad hoc tests

message ""
message "Check the edg_jobid parameter in the registered jdl"
tmp=`$COMMAND --jdl $JOBID | grep edg_jobid | awk -F= '{print $2}'| sed -e 's/\"//g' | sed -e 's/;//'`
JID=$(trim "$tmp")

if [[ $JID != $JOBID ]] ; then
  exit_failure "The registered jdl has not the correct JOBID: $JID"
else
  verbose " -> Check success"
fi

message ""
message "Check the expiration time of the delegation"
dexp=`$COMMAND -c $CONFIG_FILE $DELEGATION_OPTIONS | grep -m 1 Expiration | awk -F" : " '{print $2}'`
pexp=`$COMMAND -p $JOBID | grep -m 1 Expiration | awk -F" : " '{print $2}'`

if [[ $dexp != $pexp ]] ; then
  exit_failure "The delegation proxy expires $dexp instead the user proxy expires $pexp"
else
  verbose " -> Check success"
fi



# ... terminate

exit_success
