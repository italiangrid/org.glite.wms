#!/bin/sh

###############################################################################
#
# A job submit - wait - get output test using --resource option.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... startup
. $(dirname $0)/functions.sh
prepare

# ... print help if requested
if [ "$1" == "--help" ]; then
  echo "Usage: "
  echo ""
  echo $(basename $0) "[-d] [<CEId>]"
  echo ""
  echo " -d    use glite-wms-delegate-proxy (default behaviour is to use automatic delegation)"
  echo " CEId  Computing Element to submit job to (default behaviour is to find one automatically using glite-wms-job-list-match)"
  echo ""
  exit 0
fi

# ... define delegation parameters
if [ "$1" == "-d" ]; then
  define_delegation
  shift
fi

# ... find out valid CE IDs
if [[ "$1" == *:*/* ]]; then
   CEID=$1
   myecho "CE Id given in command line: $CEID"
else 
   myecho "Will find a suitable CE with glite-wms-job-list-match"
   run_command glite-wms-job-list-match $DELEGATION_OPTIONS --output $OUTPUTFILE $JDLFILE
   run_command cat $OUTPUTFILE
   CEID=$(awk -F ' ' '/:[[:digit:]]*\// {print $NF; exit}' $OUTPUTFILE)
   myecho "CE Id: $CEID"
fi

# ... submit a job to the first CE in the list
run_command glite-wms-job-submit $DELEGATION_OPTIONS --resource $CEID --output $TMPJOBIDFILE $JDLFILE
run_command cat $TMPJOBIDFILE
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
