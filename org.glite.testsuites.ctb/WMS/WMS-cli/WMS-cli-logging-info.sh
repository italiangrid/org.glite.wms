#!/bin/sh

###############################################################################
#
# A glite-wms-job-logging-info test
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

START=$(date +%H:%M)

# ... submit a job

run_command glite-wms-job-submit $DELEGATION_OPTIONS --output $TMPJOBIDFILE $TRUEJDL
extract_jobid $TMPJOBIDFILE
rm -f $TMPJOBIDFILE
echo $JOBID > $TMPJOBIDFILE

run_command glite-wms-job-logging-info $JOBID

run_command glite-wms-job-logging-info --config $CONFIG_FILE $JOBID
run_command glite-wms-job-logging-info --input $TMPJOBIDFILE
run_command glite-wms-job-logging-info --output $OUTPUTFILE $JOBID
run_command cat $OUTPUTFILE
rm -f $OUTPUTFILE

run_command glite-wms-job-logging-info --logfile $LOGFILE $JOBID
run_command cat $LOGFILE
rm -f $LOGFILE

run_command glite-wms-job-logging-info --noint --input $TMPJOBIDFILE --output $OUTPUTFILE --logfile $LOGFILE --config $CONFIG_FILE
run_command cat $LOGFILE
run_command cat $OUTPUTFILE
rm -f $LOGFILE
rm -f $OUTPUTFILE

NOWHOUR=$(date +%H)
NOWMIN=$(date +%M)
((NOWMIN=10#$NOWMIN+1))					# increment (force decimal system to be used)
if [[ $NOWMIN -lt 60 ]]; then
  NOW=$(date +%H:%M --date ${NOWHOUR}:${NOWMIN})	# format date, preserve leading zeros (e.g. 07:07)
else
  NOW=23:59
fi

run_command glite-wms-job-logging-info --to $NOW $JOBID	# this test fails on gLite UI 3.2.2
run_command glite-wms-job-logging-info --from $START $JOBID

run_command glite-wms-job-logging-info --event ACCEPTED $JOBID
run_command glite-wms-job-logging-info --exclude ACCEPTED $JOBID

run_command glite-wms-job-logging-info --noint --input $TMPJOBIDFILE --output $OUTPUTFILE --logfile $LOGFILE --config $CONFIG_FILE --verbosity 3 --from $START --to 23:59 --event EnQueued
run_command cat $LOGFILE
run_command cat $OUTPUTFILE

# ... terminate

exit_success
