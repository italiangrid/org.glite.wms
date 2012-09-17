#!/bin/sh
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
# AUTHORS: Gianni Pucciani, CERN
#
##############################################################################

#This script allows to run the tests automatically for different VOs.
#In this case using the test_user_1 credentials we run the tests
#for dteam and org.glite.voms-test 
#

showUsage ()
{
 echo "                                           "
 echo "Usage:  DM-certtest-all.sh "
 echo "                                           "
}

exitFailure ()
{
exit -1
}

#######################
#Parsing the arguments#
#######################
if [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$1" = "--help" ] || [ $# -gt 0 ]; then
  showUsage
  exit 2
fi

###################################
# Check for environment variables #
###################################

if [ ! -e "DM-certconfig-dteam" ]; then
  echo "The file ./DM-certconfig-dteam must be sourced in order to run the tests"
  exitFailure
fi

if [ ! -e "DM-certconfig-vomstest" ]; then
  echo "The file ./DM-certconfig-vomstest must be sourced in order to run the tests"
  exitFailure
fi

if [ ! -e "DM-certconfig-dteam-withCastor" ]; then
  echo "The file ./DM-certconfig-dteam-withCastor must be sourced in order to run the tests"
  exitFailure
fi


#########
# START #
#########

echo "**Launching DM-certtest with multiple VOs**"

########################
# Launch all the tests #
########################

declare -a tests_failed
failed=no

oldproxy=$X509_USER_PROXY

#######test_user_1 org.glite.voms-test VO without Castor##########

#test org.glite.voms-test VO without Castor
expect get-vomstest-proxy.exp >/dev/null
if [ $? != 0 ]; then
  echo "Error while getting test user certificates with expect get-vomstest-proxy.exp"
  exitFailure
fi
cp=`pwd`
export X509_USER_PROXY=$cp/vomstest-proxy

echo "DEBUG proxy is $X509_USER_PROXY"

#echo "Running DM-certtest for test_user_1 VO=org.glite.voms-test"
./DM-certtest.sh -f ./DM-certconfig-vomstest
if [ $? != 0 ];then
  failed=yes
fi

#######test_user_1 dteam VO without Castor##########
#test dteam VO without Castor
source ./DM-certconfig-dteam
expect get-dteam-proxy.exp >/dev/null

if [ $? != 0 ]; then
  echo "Error while getting test user certificates with expect get-dteam-proxy.exp"
  exitFailure
fi
cp=`pwd`
export X509_USER_PROXY=$cp/dteam-proxy

echo "DEBUG proxy is $X509_USER_PROXY"

echo "Running DM-certtest for test_user_1 VO=dteam without Castor"
./DM-certtest.sh -f DM-certconfig-dteam
if [ $? != 0 ];then
  failed=yes
fi

#######normal user dteam VO with Castor##########
#set back the old user proxy
export X509_USER_PROXY=$oldproxy

echo "DEBUG proxy is $X509_USER_PROXY"

#test dteam VO including Castor
echo "Running DM-certtest for VO=dteam with Castor"
./DM-certtest.sh -f DM-certconfig-dteam-withCastor
if [ $? != 0 ];then
  failed=yes
fi

echo
#Analyse results
if [ "x$failed" == "xyes" ];then
  echo "OVERALL TEST FAILED"
  exit -1
fi

echo "OVERALL TEST PASSED"
exit 0

