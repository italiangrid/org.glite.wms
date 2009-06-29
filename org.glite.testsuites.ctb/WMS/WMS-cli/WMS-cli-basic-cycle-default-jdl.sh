#!/bin/sh

###############################################################################
#
# A job submit - wait - get output test using --default-jdl option
# of glite-wms-job-submit (present in gLite >= 3.1).
#
# It is assumed that the parameters given in the default jdl take precedence
# over the "main" JDL (like it is in gLite 3.1)
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

# ... make a test file
echo -n "test file created on " > $TESTFILE
date >> $TESTFILE
ls -l $TESTFILE
run_command cat $TESTFILE

# ... make a test JDL file
echo 'Executable = "/bin/cat";' >  $TMPJDLFILE
echo 'Arguments = "test.file";' >> $TMPJDLFILE
echo "InputSandbox = {\"$TESTFILE\"};"   >> $TMPJDLFILE
run_command cat $TMPJDLFILE

# ... submit a job

run_command glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --output $TMPJOBIDFILE --default-jdl $TMPJDLFILE $JDLFILE
extract_jobid $TMPJOBIDFILE

run_command_keep_output glite-wms-job-status --verbosity 3 $JOBID

i=1

# ... wait until job is done or time is out

while ! is_finished "$OUTPUT"
do

  if [ $i -ge $NUM_STATUS_RETRIEVALS ]; then
    exit_timeout $JOBID
  fi
  
  myecho "sleeping ($i) ..."
  sleep $SLEEP_TIME

  run_command_keep_output glite-wms-job-status $JOBID

  ((i++))

done

# ... show job status

run_command glite-wms-job-status --verbosity 3 $JOBID

# ... get job output in a temporary directory

run_command glite-wms-job-output --dir $JOB_OUTPUT_DIR $JOBID

# ... list the directory and print out its content

run_command ls -l $JOB_OUTPUT_DIR
run_command cat ${JOB_OUTPUT_DIR}/std.out
run_command cat ${JOB_OUTPUT_DIR}/std.err

# ... terminate

exit_success
