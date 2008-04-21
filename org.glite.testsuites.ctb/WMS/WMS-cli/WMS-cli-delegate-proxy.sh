#!/bin/sh

###############################################################################
#
# A glite-wms-job-delegate-proxy test
#
# This test is intended to verify glite-wms-job-delegate-proxy command with
# various options. It does not check whether the resulting proxy is actually
# usable for job submission. In order to test glite-wms-job-delegate-proxy
# in conjunction with other commands use -d option of the other test scripts.
# E.g. WMS-CLI-basic-cycle.sh -d. (Special options of glite-wms-job-delegate-proxy
# will not be tested then.)
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare

# ... try glite-wms-job-delegate-proxy with --autm-delegation

run_command glite-wms-job-delegate-proxy --output $OUTPUTFILE --autm-delegation
run_command cat $OUTPUTFILE

# ... determine delegation Id and service endpoint

DELEGATIONID=$(awk -F ' ' '/delegation/ {print $NF}' $OUTPUTFILE)
ENDPOINT=$(awk -F ' ' '/WMProxy/ {print $NF}' $OUTPUTFILE)
myecho "Delegation Id   : $DELEGATIONID"
myecho "WMProxy endpoint: $ENDPOINT"
rm -f $OUTPUTFILE

# ... try with minimum options

run_command glite-wms-job-delegate-proxy -d $DELEGATIONID

# ... try with --logfile

run_command glite-wms-job-delegate-proxy --logfile $LOGFILE -d $DELEGATIONID
run_command cat $LOGFILE
rm -f $LOGFILE

# ... try with --config

run_command glite-wms-job-delegate-proxy --config $CONFIG_FILE -d $DELEGATIONID

# ... try with --endpoint

run_command glite-wms-job-delegate-proxy --endpoint $ENDPOINT -d $DELEGATIONID

# ... create empty files (--noint test)

echo "" > $OUTPUTFILE
echo "" > $LOGFILE

# ... test many options together

run_command glite-wms-job-delegate-proxy --noint --output $OUTPUTFILE --logfile $LOGFILE --config $CONFIG_FILE --endpoint $ENDPOINT -d $DELEGATIONID
run_command cat $LOGFILE
run_command cat $OUTPUTFILE

# ... terminate

exit_success
