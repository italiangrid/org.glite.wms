#!/bin/sh

###############################################################################
#
# A job submit - wait - get output test using --input option.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

# ... Find out valid CE IDs

run_command glite-wms-job-list-match $DELEGATION_OPTIONS --output $OUTPUTFILE $JDLFILE
run_command cat $OUTPUTFILE
awk -F ' ' '/:[[:digit:]]*\// {print $NF}' $OUTPUTFILE > $CEIDFILE
myecho "CE Ids: "
cat $CEIDFILE

# ... submit a job to the first CE in the list

run_command glite-wms-job-submit $DELEGATION_OPTIONS --noint --input $CEIDFILE --output $TMPJOBIDFILE $JDLFILE
run_command cat $TMPJOBIDFILE
extract_jobid $TMPJOBIDFILE
#rm -f $TMPJOBIDFILE
#echo $JOBID > $TMPJOBIDFILE

run_command_keep_output glite-wms-job-status --input $TMPJOBIDFILE --verbosity 2

i=1

# ... wait until job is done or time is out

while ! is_finished "$OUTPUT"
do

  if [ $i -ge $NUM_STATUS_RETRIEVALS ]; then
    exit_timeout $JOBID
  fi
  
  myecho "sleeping ($i) ..."
  sleep $SLEEP_TIME

  run_command_keep_output glite-wms-job-status --input $TMPJOBIDFILE

  ((i++))

done

# ... show job status

run_command glite-wms-job-status --input $TMPJOBIDFILE --verbosity 2

# ... get job output in a temporary directory

run_command glite-wms-job-output --input $TMPJOBIDFILE --dir $JOB_OUTPUT_DIR

# ... list the directory and print out its content

run_command ls -l $JOB_OUTPUT_DIR
run_command cat ${JOB_OUTPUT_DIR}/std.out
run_command cat ${JOB_OUTPUT_DIR}/std.err

# ... terminate

exit_success
