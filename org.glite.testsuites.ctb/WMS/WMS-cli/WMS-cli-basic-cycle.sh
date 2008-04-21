#!/bin/sh

###############################################################################
#
# A basic job submit - wait - get output test.
#
# Features: The test will fail when one of the tested commands fails,
# but not when the job itself finishes with a failure or aborted in the queue.
# A success will also be returned in case the job did not finish, or even start,
# within certain period of time, providing that it could be successfully cancelled.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

# ... submit a job

run_command glite-wms-job-submit $DELEGATION_OPTIONS --output $TMPJOBIDFILE $JDLFILE
extract_jobid $TMPJOBIDFILE

# ... wait loop with job-status calls

wait_until_job_finishes

# ... get job output in a temporary directory

run_command glite-wms-job-output --dir $JOB_OUTPUT_DIR $JOBID

# ... list the directory and print out its content

run_command ls -l $JOB_OUTPUT_DIR
run_command cat ${JOB_OUTPUT_DIR}/std.out
run_command cat ${JOB_OUTPUT_DIR}/std.err

# ... terminate

exit_success
