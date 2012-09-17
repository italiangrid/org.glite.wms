#!/bin/sh

###############################################################################
#
# CREAM-CLI suite test: it runs all the test in the right order.
#
# Author: Alessio Gianelle <sa3-italia@mi.infn.it>
# Version: $Id:
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@

BS=`echo $CREAM | awk -F"/" '{print $2}' | awk -F"-" '{print $2}'`


if [[ $BS == *pbs* ]] || [ $BS == *torque* ] ; then
	TESTLIST="./CREAM-cli-delegation.sh ./CREAM-cli-delegation-renew.sh ./CREAM-cli-job-submit.sh ./CREAM-cli-job-status-simple.sh ./CREAM-cli-job-status-filtered.sh ./CREAM-cli-job-output.sh ./CREAM-cli-job-cancel.sh ./CREAM-cli-job-purge.sh ./CREAM-cli-job-list.sh ./CREAM-cli-submission-management.sh ./CREAM-cli-event-query.sh ./CREAM-cli-service-info.sh"
else
	TESTLIST="./CREAM-cli-delegation.sh ./CREAM-cli-delegation-renew.sh ./CREAM-cli-job-submit.sh ./CREAM-cli-job-status-simple.sh ./CREAM-cli-job-status-filtered.sh ./CREAM-cli-job-output.sh ./CREAM-cli-job-cancel.sh ./CREAM-cli-job-suspend.sh ./CREAM-cli-job-resume.sh ./CREAM-cli-job-purge.sh ./CREAM-cli-job-list.sh ./CREAM-cli-submission-management.sh ./CREAM-cli-event-query.sh ./CREAM-cli-service-info.sh"
fi 

FAILED=0

for command in $TESTLIST; do
	run_command $command $@
	if [ $? -eq 0 ] ; then
  	success
	else
  	my_echo " >>> TEST NOT PASSED <<<" 
  	((FAILED++)) # continue
	fi
done

if [ $FAILED -ne 0 ] ; then
	my_echo " !!! $FAILED test(s) failed !!!"
else
	my_echo " <<< All tests PASSED >>>"
fi

cleanup
exit $FAILED
