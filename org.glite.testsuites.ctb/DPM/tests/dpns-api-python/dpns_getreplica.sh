#!/bin/bash
##############################################################################
# Copyright (c) Members of the EGEE Collaboration. 2004.
# See http://www.eu-egee.org/partners/ for details on the copyright
# holders.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##############################################################################
#
# AUTHORS: Dimitar Shiyachki <Dimitar.Shiyachki@cern.ch>
#
##############################################################################

export DPNS_HOME="/dpm/$CFG_DPNS_DOMAIN/$CFG_DPNS_BASEDIR/$CFG_VO"
PYTHON_FILES=$TESTSUITE_LOCATION/dpns-api-python
TEST_DIR=d`date +%s%N`
TEST_SUBDIR1=s1`date +%s%N`
TEST_SUBDIR2=s2`date +%s%N`
DUMMY_USERNAME=u`date +%s%N`
DUMMY_GROUPNAME=g`date +%s%N`
TEMP_FILE=/tmp/_dpm_test_temp

dpm_exec ()
{
   ssh root@"$DPM_HOST" "$1"
}

cleanup ()
{
   lcg-del --nolfc srm://$DPM_HOST/$DPNS_HOME/$TEST_DIR/ls-1
   dpns-rm -r $TEST_DIR
   rm -f $TEMP_FILE
}

report_failure ()
{
   echo
   echo "OVERALL RESULT: FAILURE"
   exit 1
}

echo "Creating a directory for the test: $TEST_DIR"
dpns-mkdir -p $TEST_DIR

echo "Scenario 1: Copy a file to the DPM server with lcg-cp and list the created replicas"
lcg-cp /bin/ls srm://$DPM_HOST/$DPNS_HOME/$TEST_DIR/ls-1
if [ $? -ne 0 ]; then
   echo "Error copying file with lcg-cp. Test has failed."
   cleanup
   report_failure
fi

$PYTHON_FILES/dpns_getreplica.py "$DPNS_HOME/$TEST_DIR/ls-1" >$TEMP_FILE
if [ $? -eq 1 ]; then
   echo "File not found via the dpns_getreplicax python call. Test has failed."
   cleanup
   report_failure
fi

if [ $(cat $TEMP_FILE | wc -l) -eq 0 ]; then
   echo "No replicas returned by dpns_getreplicax. Test has failed."
   cleanup
   report_failure
fi

if [ $(cat $TEMP_FILE | wc -l) -gt 1 ]; then
   echo "More than one replicas returned by dpns_getreplicax. Test has failed."
   cleanup
   report_failure
fi

if ! grep ":P:P:" $TEMP_FILE; then
   echo "The replica returned is not permanent or not primary. Test has failed."
   cleanup
   report_failure
fi

cleanup
echo
echo "OVERALL RESULT: SUCCESS"
exit 0

