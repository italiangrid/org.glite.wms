#!/bin/sh

###############################################################################
#
# A glite-(wms-)job-status test using special options such as --all
# This test requires indexing capabilities of the LB server to be enabled
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare

myecho "BEWARE: This test requires certain indexing capabilities to be enabled on the LB server!"

# ... try glite-wms-job-status

run_command glite-wms-job-status --all
run_command glite-wms-job-status --from 06:00
run_command glite-wms-job-status --from 06:00 --to 23:00
run_command glite-wms-job-status --status DONE
run_command glite-wms-job-status --exclude DONE

# ... terminate

exit_success
