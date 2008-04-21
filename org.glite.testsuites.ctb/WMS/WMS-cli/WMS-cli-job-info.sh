#!/bin/sh

###############################################################################
#
# A glite-wms-job-info test
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare

# ... delegate proxy

DELEGATION_OPTIONS="--delegationid $$"
myecho "delegating proxy ..."
run_command glite-wms-job-delegate-proxy $DELEGATION_OPTIONS

# ... submit a job

run_command glite-wms-job-submit $DELEGATION_OPTIONS --output $TMPJOBIDFILE $TRUEJDL
extract_jobid $TMPJOBIDFILE

# ... try 'glite-wms-job-info --jdl-original' with various extra options

run_command glite-wms-job-info --input $TMPJOBIDFILE --jdl-original
run_command glite-wms-job-info --config $CONFIG_FILE --jdl-original $JOBID
run_command glite-wms-job-info --output $OUTPUTFILE --jdl-original $JOBID
run_command cat $OUTPUTFILE
rm -f $OUTPUTFILE

run_command glite-wms-job-info --logfile $LOGFILE --jdl-original $JOBID
run_command cat $LOGFILE
rm -f $LOGFILE
echo "" > $LOGFILE

# ... try glite-wms-job-info --jdl-original/--jdl/--proxy/--delegationid with many options simultaneously

run_command glite-wms-job-info --noint --config $CONFIG_FILE --output $OUTPUTFILE --logfile $LOGFILE --input $TMPJOBIDFILE --jdl-original
run_command cat $OUTPUTFILE
run_command cat $LOGFILE
rm -f $OUTPUTFILE
rm -f $LOGFILE

run_command glite-wms-job-info --noint --config $CONFIG_FILE --output $OUTPUTFILE --logfile $LOGFILE --input $TMPJOBIDFILE --jdl
run_command cat $OUTPUTFILE
run_command cat $LOGFILE
rm -f $OUTPUTFILE
rm -f $LOGFILE

run_command glite-wms-job-info --noint --config $CONFIG_FILE --output $OUTPUTFILE --logfile $LOGFILE --input $TMPJOBIDFILE --proxy
run_command cat $OUTPUTFILE
run_command cat $LOGFILE
rm -f $OUTPUTFILE
rm -f $LOGFILE

run_command glite-wms-job-info --noint --config $CONFIG_FILE --output $OUTPUTFILE --logfile $LOGFILE $DELEGATION_OPTIONS
run_command cat $OUTPUTFILE
run_command cat $LOGFILE

# ... try glite-wms-job-info --delegationid/--jdl-original/--jdl/--proxy without special options

run_command glite-wms-job-info $DELEGATION_OPTIONS
run_command glite-wms-job-info --jdl-original $JOBID
run_command glite-wms-job-info --jdl $JOBID
run_command glite-wms-job-info --proxy $JOBID

# ... terminate

exit_success
