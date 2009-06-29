#!/bin/sh

###############################################################################
#
# A job submit test using --valid option of glite-wms-job-submit.
#
# This test will fail on gLite UI <= 3.1.12 (?) - see savanna bug 34420,
# https://savannah.cern.ch/bugs/?34420
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

# ... submit a jdl valid for 1 minute

run_command glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --output $TMPJOBIDFILE --valid 00:01 $TRUEJDL
run_command cat $TMPJOBIDFILE
extract_jobid $TMPJOBIDFILE

# ... wait until job is done or time is out (just to make sure the JDL was really okey)

wait_until_job_finishes

# ... terminate

exit_success
