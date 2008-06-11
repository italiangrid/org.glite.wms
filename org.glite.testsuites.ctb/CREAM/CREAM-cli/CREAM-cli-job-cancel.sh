#!/bin/sh

###############################################################################
#
# A basic job interruption operation
#
# Features: This test performs several cancel calls on the server.
#
# Author: Paolo Andreetto <sa3-italia@mi.infn.it>
# Version: $Id:
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

FAILED=0

my_echo "TEST 0: submit and then cancel a running job:"

wait_until_job_runs

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-cancel -N $JOBID
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  success
fi

my_echo "TEST 1: submit and then cancel jobs with --all option:"

wait_until_job_runs

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-cancel -N --all -e $ENDPOINT
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  success
fi

my_echo "TEST 2: try to cancel a terminated job:"

wait_until_job_finishes

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-cancel -N $JOBID
RESULT=`echo ${COM_OUTPUT} | grep "status not compatible with the JOB_CANCEL command"`
if [ -z "$RESULT" ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  success
fi

if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 3 differents tests"
else
  exit_success
fi

