#!/bin/sh

###############################################################################
#
# Basic test of glite-wms-job-perusal (getting output from job at run time).
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

if [ ! -f perusal.jdl ] || [ ! -f sleeper.sh ]; then
  myecho "Warning: this test needs perusal.jdl and sleeper.sh to be in current directory"
  myecho "Hint: try cd $(dirname $0); ./$(basename $0)"
  exit_failure
fi

# ... submit a job

run_command glite-wms-job-submit $DELEGATION_OPTIONS --output $TMPJOBIDFILE perusal.jdl 
extract_jobid $TMPJOBIDFILE

# ... enable file perusal for out.txt

run_command glite-wms-job-perusal --set -f out.txt $JOBID

# ... wait until job is done or time is out, request out.txt every time

run_command_keep_output glite-wms-job-status --input $TMPJOBIDFILE --verbosity 2

i=1

while ! is_finished "$OUTPUT"
do

  if [ $i -ge $NUM_STATUS_RETRIEVALS ]; then
    exit_timeout $JOBID
  fi

  myecho "sleeping ($i) ..."
  sleep 10

  run_command_keep_output glite-wms-job-status --verbosity 0 $JOBID

  run_command glite-wms-job-perusal --get -f out.txt --dir $JOB_OUTPUT_DIR $JOBID

  ((i++))

done

# ... print final status, pay attention to things related to file perusal

echo ""
run_command_keep_output glite-wms-job-status --input $TMPJOBIDFILE --verbosity 3
myecho "Strings related to file perusal (from job-status output):"
echo "$OUTPUT" | grep "Perusal"
echo ""

# ... get job output in a temporary directory

run_command glite-wms-job-output --nosubdir --noint --dir $JOB_OUTPUT_DIR $JOBID

# ... list the directory and print out its content

run_command ls -l $JOB_OUTPUT_DIR
run_command cat ${JOB_OUTPUT_DIR}/std.out
run_command cat ${JOB_OUTPUT_DIR}/std.err
run_command cat ${JOB_OUTPUT_DIR}/out.txt

# ... print out the out.txt* files

myecho "list of out.txt parts:"
if ls ${JOB_OUTPUT_DIR}/out.txt-*; then
   myecho "concatenated out.txt:"
   cat ${JOB_OUTPUT_DIR}/out.txt-*
else
   echo ""
   myecho "PROBLEM: No output seem to be obtained using glite-wms-job-perusal !"
   exit_failure
fi

# ... terminate

myecho "No errors have been encountered, but please have a look at the output of job-perusal to make sure."

exit_success
