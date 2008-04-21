#!/bin/sh

###############################################################################
#
# A job submit test using --to option of glite-wms-job-submit
#
# NB: the job may be aborted with Status Reason = "request expired". This is not a error.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

# ... make a timestamp which is 1 min (max) from now

NOWHOUR=$(date +%H)
NOWMIN=$(date +%M)
((NOWMIN=10#$NOWMIN+1))                                 # increment (force decimal system to be used)
if [[ $NOWMIN -lt 60 ]]; then
  NOW=$(date +%H:%M --date ${NOWHOUR}:${NOWMIN})        # format date, preserve leading zeros (e.g. 07:07)
else
  NOW=23:59
fi

# ... submit a jdl valid for max 1 minute from NOW

run_command glite-wms-job-submit $DELEGATION_OPTIONS --output $TMPJOBIDFILE --to $NOW $TRUEJDL
run_command cat $TMPJOBIDFILE
extract_jobid $TMPJOBIDFILE

# ... wait until job is done or time is out (just to make sure the JDL was really okey)

wait_until_job_finishes

# ... terminate

exit_success
