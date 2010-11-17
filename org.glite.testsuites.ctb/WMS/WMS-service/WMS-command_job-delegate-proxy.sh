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
# The following tests are possible only if test option -n is not abilitate
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
  verbose "Verify the delegation"
  run_command glite-wms-job-info -d $1 --config $CONFIG_FILE
  verbose "Try a submit"
  run_command glite-wms-job-submit -d $1 --config $CONFIG_FILE --nomsg $JDLFILE
  JOBID=$OUTPUT
  verbose "Cancel the unused job"
  run_command glite-wms-job-cancel --noint $JOBID
}


COMMAND=glite-wms-job-delegate-proxy
message "Check if command $COMMAND exists"
run_command which $COMMAND

# Test --version option
message ""
message "Test --version option"
run_command $COMMAND --version
verbose "Check the output command"
run_command grep "\"WMS User Interface version\"" <<< "$OUTPUT"
message "We are testing $OUTPUT"

# test option --autm-delegation
message ""
message "Test option --autm-delegation"
run_command $COMMAND --config $CONFIG_FILE --output $OUTPUTFILE --autm-delegation
DELEGATIONID=$(awk -F ' ' '/delegation/ {print $NF}' $OUTPUTFILE)
verbose "Delegation Id: $DELEGATIONID"
checkdeleg $DELEGATIONID
remove $OUTPUTFILE

# use a predefined delegation name (option -d)
message ""
message "Test option -d"
DELEGATIONID="deleg-$$"
run_command $COMMAND --config $CONFIG_FILE  -d $DELEGATIONID
checkdeleg $DELEGATIONID

# test option --logfile
message ""
message "Test option --logfile"
run_command $COMMAND --config $CONFIG_FILE  --logfile $LOGFILE -d $DELEGATIONID
verbose "Check logfile"
run_command cat $LOGFILE
checkdeleg $DELEGATIONID
remove $LOGFILE

# test option --endpoint
message ""
message "Test option --endpoint"
run_command $COMMAND --config $CONFIG_FILE  --endpoint  https://${WMS}:7443/glite_wms_wmproxy_server -d $DELEGATIONID --output $OUTPUTFILE
ENDPOINT=$(awk -F ' ' '/WMProxy/ {print $NF}' $OUTPUTFILE)
if [ $ENDPOINT != "https://${WMS}:7443/glite_wms_wmproxy_server" ] ; then
  exit_failure "Wrong endpoint delegating proxy"
fi
checkdeleg $DELEGATIONID
remove $OUTPUTFILE

# test all options
echo "" > $OUTPUTFILE
echo "" > $LOGFILE
message ""
message "Test all options together"
run_command $COMMAND --noint --output $OUTPUTFILE --logfile $LOGFILE --config $CONFIG_FILE --endpoint $ENDPOINT -d $DELEGATIONID
verbose "Check the output"
run_command cat $OUTPUTFILE
verbose "Check the logfile"
run_command cat $LOGFILE
checkdeleg $DELEGATIONID
remove $LOGFILE
remove $OUTPUTFILE

# for the next tests we need user proxy password
if [ $NOPROXY -eq 0 ] ; then 
  # create a shorter delegation
  set_proxy "$PROXY" "00:10"
  message ""
  message "Try to delegate with a short proxy and check the validity"
  run_command $COMMAND --config $CONFIG_FILE  -d $DELEGATIONID
  verbose "Check the delegation timeleft value"
  run_command glite-wms-job-info -d $DELEGATIONID --config $CONFIG_FILE --output $OUTPUTFILE
  text=`grep -m 1 Timeleft $OUTPUTFILE | awk '{print $4}'`
  value=`grep -m 1 Timeleft $OUTPUTFILE | awk '{print $3}'`
  remove $OUTPUTFILE  
  if [ $text != "min" ] || [ $value -gt 10 ] ; then
    exit_failure "Wrong timeleft delegating proxy"
  fi
  checkdeleg $DELEGATIONID

  # try with an expiry delegation   
  set_proxy "$PROXY" "00:01"
  message ""
  message "Works with expiring proxy..." 
  run_command $COMMAND --config $CONFIG_FILE  -d $DELEGATIONID
  verbose "Wait until proxy expired..."
  run_command sleep 60
  # first try to delegate again with an expired proxy
  message "Try to delegate with an expired proxy"
  run_command_fail $COMMAND --config $CONFIG_FILE --autm-delegation
  # refresh the proxy
  set_proxy "$PROXY"
  # then check if the old delegation is expired
  verbose "Check if the old delegation is expired"
  run_command_fail glite-wms-job-info  --config $CONFIG_FILE -d $DELEGATIONID | grep Timeleft
  # then try to submit with the expired delegation
  message "Try to submit with an expired delegation"
  run_command_fail glite-wms-job-submit -d $DELEGATIONID --config $CONFIG_FILE --logfile $LOGFILE $JDLFILE
  verbose "Check the output of the command"
  run_command "grep \"The delegated Proxy has expired\" $LOGFILE"
  remove $LOGFILE
fi

# ... terminate

exit_success
