#!/bin/sh

###############################################################################
#
# Test a complete job cycle: submission -> get-output
#
# Test submission of a normal job to an LCG-CE
# Test submission of a normal job to a CREAM CE
#
# We need to test submission of various type of jobs (TBD)
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

function checkstatus()
{
  sleep 5
  verbose "Check if the status is correct"
  grep "\"Warning - JobPurging not allowed\"" $TMPFILE
    if [[ $? -eq 1 ]] ; then
      get_STATUS $1
      if [[ "$JOBSTATUS" != *Cleared* ]]; then
        message "WARNING Job $1 not cleared!"
        return 1
      fi
    else
      verbose "Warning: WMS is not recognized by the LB, JobPurging not allowed!"
    fi
}

function checkoutput()
{
  verbose "Check if the output files are correctly retrieved"
  run_command "grep -A 1 'have been successfully retrieved' $TMPFILE | grep -v 'have been successfully retrieved'"
  dir=$OUTPUT
  verbose "Look if in $dir there are the output files std.out and std.err"
  run_command ls ${dir}/std.out
  run_command ls ${dir}/std.err
}

function check()
{
  echo "$OUTPUT" > $TMPFILE
  checkstatus $1
  if [[ $? -eq 0 ]] ; then
    checkoutput
  fi
}

prepare "test submission cycle" $@

message ""
message "Submit a normal job to an LCG-CE"
set_isbjdl $JDLFILE
set_requirements "!RegExp(\"8443/cream-\",other.GlueCEUniqueID)"
run_command glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg $JDLFILE
JOBID=$OUTPUT
sleep 10
get_CE $JOBID
verbose "Check if it match a correct CE"
run_command grep "2119/jobmanager" <<< "$CENAME"
wait_until_job_finishes $JOBID
get_STATUS $JOBID
if [[ "$JOBSTATUS" == "Done (Success)" ]]; then
  verbose "Retrieve the output"
  run_command glite-wms-job-output $JOBID
  check $JOBID
else
  message "WARNING Job finishes $JOBSTATUS, cannot retrieve output"
fi


message ""
message "Submit a normal job to a CREAM-CE"
set_isbjdl $JDLFILE
set_requirements "RegExp(\"8443/cream-\",other.GlueCEUniqueID)"
run_command glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg $JDLFILE
JOBID=$OUTPUT
sleep 10
get_CE $JOBID
verbose "Check if it match a correct CE"
run_command grep "8443/cream" <<< "$CENAME"
wait_until_job_finishes $JOBID
get_STATUS $JOBID
if [[ "$JOBSTATUS" == "Done (Success)" ]]; then
  verbose "Retrieve the output"
  run_command glite-wms-job-output $JOBID
  check $JOBID
else
  message "WARNING Job finishes $JOBSTATUS, cannot retrieve output"
fi
	


# ... terminate

exit_success
