#!/bin/sh

###############################################################################
#
# A basic job resume operation
#
# Features: This test performs several resume calls on the server.
#
# Author: Paolo Andreetto <sa3-italia@mi.infn.it>
# Version: $Id:
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

FAILED=0

my_echo "TEST 0: submit, suspend and then resume a running job:"

wait_until_job_runs

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-suspend -N $JOBID
if [ $? -ne 0 ]; then
  failure ${COM_OUTPUT}
  ((FAILED++)) # continue
else
  sleep 10
  extract_status $JOBID
  if [ "$JOBSTATUS" == "HELD" ]; then
    run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-resume -N $JOBID
    if [ $? -ne 0 ]; then
      failure ${COM_OUTPUT}
      ((FAILED++)) # continue
    else
      success
    fi
  else
    failure "The job is not suspended, status is $JOBSTATUS"
    ((FAILED++)) # continue
  fi

fi





if [ $FAILED -gt 0 ] ; then
  exit_failure "$FAILED test(s) failed on 3 differents tests"
else
  exit_success
fi
