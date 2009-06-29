#!/bin/sh

###############################################################################
#
# A job submit - cancel test with the focus on glite-wms-job-cancel
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

function submit_job()
{
  rm -f $TMPJOBIDFILE
  run_command glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --output $TMPJOBIDFILE $TRUEJDL
  extract_jobid $TMPJOBIDFILE
  #rm -f $TMPJOBIDFILE
  #echo $JOBID > $TMPJOBIDFILE
}

# ... startup

. $(dirname $0)/functions.sh

prepare $@

# ... submit a job - cancel job. with different options

submit_job
run_command glite-wms-job-cancel --config $CONFIG_FILE --noint $JOBID

submit_job
run_command glite-wms-job-cancel --config $CONFIG_FILE --noint --input $TMPJOBIDFILE

submit_job
run_command glite-wms-job-cancel --config $CONFIG_FILE --noint --output $OUTPUTFILE $JOBID
run_command cat $OUTPUTFILE

submit_job
run_command glite-wms-job-cancel --config $CONFIG_FILE --noint --logfile $LOGFILE $JOBID
run_command cat $LOGFILE

submit_job
run_command glite-wms-job-cancel --noint --config $CONFIG_FILE $JOBID

submit_job
run_command glite-wms-job-cancel --noint --input $TMPJOBIDFILE --output $OUTPUTFILE --logfile $LOGFILE --config $CONFIG_FILE
run_command cat $LOGFILE
run_command cat $OUTPUTFILE

# ... show job status

#sleep 5
#run_command glite-wms-job-status --verbosity 2 $JOBID

# ... terminate

exit_success
