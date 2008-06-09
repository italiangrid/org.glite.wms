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

printf "[
JobType = \"Normal\";
Executable = \"/bin/sleep\";
Arguments = \"600\";
StdOuput=\"out.txt\";
StdError=\"err.txt\";
]
" > ${MYTMPDIR}/long_sleep.jdl

my_echo "TEST 0: submit and then cancel a running job:"
run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-submit -a -r $CREAM ${MYTMPDIR}/long_sleep.jdl
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
fi

extract_jobid ${COM_OUTPUT}
my_echo "Submitted job: $JOBID"

ok=0
while [ "$ok" == "0" ]; do

  my_echo "Waiting for the running job (30s)"
  sleep 30

  extract_status $JOBID

  if [ "$JOBSTATUS" == "RUNNING" -o "$JOBSTATUS" == "REALLY-RUNNING" ]; then
    ok=1
  fi
  if [ "$JOBSTATUS" == "DONE-FAILED" -o "$JOBSTATUS" == "ABORTED" ]; then
    exit_failure "Cannot run the job (${JOBSTATUS})"
  fi
done

run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-cancel -N $JOBID
if [ $? -ne 0 ]; then
  exit_failure ${COM_OUTPUT}
else
  success
fi

exit_success

