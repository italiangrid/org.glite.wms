#!/bin/sh

###############################################################################
#
# A job submit - wait - get output test using --register-only --transfer-files,
# and --start options of glite-wms-job-submit, and --list-only option of 
# glite-wms-job-output.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... start

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
echo 'StdOutput  = "std.out";'  >> $TMPJDLFILE
echo 'StdError   = "std.err";'  >> $TMPJDLFILE
echo "InputSandbox = {\"$TESTFILE\"};"   >> $TMPJDLFILE
echo 'OutputSandbox = {"std.out","std.err"};' >> $TMPJDLFILE
run_command cat $TMPJDLFILE

# ... submit a job

run_command glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --register-only --transfer-files --proto gsiftp --output $TMPJOBIDFILE $TMPJDLFILE
extract_jobid $TMPJOBIDFILE
run_command glite-wms-job-submit --start $JOBID

# ... wait loop with job-status calls

wait_until_job_finishes

# ... get job output in a temporary directory

run_command glite-wms-job-output --list-only $JOBID
run_command glite-wms-job-output --nosubdir --dir $JOB_OUTPUT_DIR --noint $JOBID

# ... list the directory and print out its content

run_command ls -l $JOB_OUTPUT_DIR
run_command cat ${JOB_OUTPUT_DIR}/std.out
run_command cat ${JOB_OUTPUT_DIR}/std.err

# ... terminate

exit_success
