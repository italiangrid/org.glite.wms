#!/bin/sh

###############################################################################
#
# A job submit - wait - get output test using --config option.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

# ... submit a job

run_command glite-wms-job-submit $DELEGATION_OPTIONS --output $TMPJOBIDFILE --config $CONFIG_FILE $JDLFILE
extract_jobid $TMPJOBIDFILE

run_command_keep_output glite-wms-job-status --verbosity 2 --config $CONFIG_FILE $JOBID

i=1

# ... wait until job is done or time is out

while ! is_finished "$OUTPUT"
do

  if [ $i -ge $NUM_STATUS_RETRIEVALS ]; then
    exit_timeout $JOBID
  fi
  
  myecho "sleeping ($i) ..."
  sleep $SLEEP_TIME

  run_command_keep_output glite-wms-job-status --config $CONFIG_FILE $JOBID

  ((i++))

done

# ... show detailed job status

run_command glite-wms-job-status --verbosity 2 --config $CONFIG_FILE $JOBID

# ... get job output in a temporary directory

run_command glite-wms-job-output --nosubdir --dir $JOB_OUTPUT_DIR --config $CONFIG_FILE $JOBID

# ... list the directory and print out its content

run_command ls -l $JOB_OUTPUT_DIR
run_command cat ${JOB_OUTPUT_DIR}/std.out
run_command cat ${JOB_OUTPUT_DIR}/std.err

# ... terminate

exit_success
