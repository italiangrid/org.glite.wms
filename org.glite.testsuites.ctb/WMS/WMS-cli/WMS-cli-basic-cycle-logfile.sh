#!/bin/sh

###############################################################################
#
# A job submit - wait - get output test using --logfile option.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

# ... submit a job

run_command glite-wms-job-submit $DELEGATION_OPTIONS --output $TMPJOBIDFILE --logfile $LOGFILE --nomsg $JDLFILE
extract_jobid $TMPJOBIDFILE
run_command cat $LOGFILE
rm -f $LOGFILE

# ... get status

run_command_keep_output glite-wms-job-status --verbosity 2 --logfile $LOGFILE $JOBID
run_command cat $LOGFILE
rm -f $LOGFILE

i=1

# ... wait until job is done or time is out

while ! is_finished "$OUTPUT"
do

  if [ $i -ge $NUM_STATUS_RETRIEVALS ]; then
    exit_timeout $JOBID
  fi
  
  myecho "sleeping ($i) ..."
  sleep $SLEEP_TIME

  run_command glite-wms-job-status --output $LOGFILE $JOBID
  run_command cat $LOGFILE
  OUTPUT=$(cat $LOGFILE)
  rm -f $LOGFILE

  ((i++))

done

# ... show detailed job status

run_command glite-wms-job-status --noint --verbosity 2 --logfile $LOGFILE $JOBID
run_command cat $LOGFILE
rm -f $LOGFILE

# ... get job output in a temporary directory

run_command glite-wms-job-output --dir $JOB_OUTPUT_DIR --logfile $LOGFILE $JOBID
run_command cat $LOGFILE
rm -f $LOGFILE

# ... list the directory and print out its content

run_command ls -l $JOB_OUTPUT_DIR
run_command cat ${JOB_OUTPUT_DIR}/std.out
run_command cat ${JOB_OUTPUT_DIR}/std.err

# ... terminate

exit_success
