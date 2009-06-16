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

showUsage ()
{
 echo "Usage: $0 "
}

exitFailure ()
{
echo "------------------------------------------------"
echo "END `date`"
echo "-TEST FAILED-"
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

if [ -e "myproxy-certconfig" ]; then
  source ./myproxy-certconfig
else
  echo "The file ./myproxy-certconfig must be sourced in order to run the tests"
  exit Failure
fi

if [ -z "$MYPROXY_HOST" ]; then
  echo "You need to set MYPROXY_HOST in myproxy-certconfig in order to run the tests"
  exitFailure
else
  hostname=$MYPROXY_HOST
fi

####################################
# Checking if there is valid proxy #
####################################

ProxyExist=`voms-proxy-info 2>/dev/null | grep timeleft | wc -l`

ProxyExpired=`voms-proxy-info 2>/dev/null | grep  "timeleft  : 0:00:00" | wc -l`

if [ $ProxyExist -gt 0 -a $ProxyExpired -eq 0 ]; then
  #nop
  :
else
  echo "Valid proxy is needed for this test!"
  if [ $ProxyExpired -gt 0 ]; then
    echo "Proxy credential expired!"
  fi
  exitFailure
fi

########################
# Launch all the tests #
########################

declare -a tests_failed
failed=no

testdir=./tests
tests_list=(MyProxy-register.sh MyProxy-basic)

pushd $testdir >> /dev/null

touch testfile 2> /dev/null
if [ $? -ne 0 ]; then
  echo "myproxy tests directory is not writable, if you are on AFS be sure to have a valid token"
  exitFailure
fi

for item in ${tests_list[*]}
do
  rm -rf ${item}_result.txt testfile
  echo "Executing $item"
  ./$item $hostname > ${item}_result.txt
  grep '\-TEST FAILED\-' ${item}_result.txt >> /dev/null
  if [ "$?" = 0 ]; then
    echo "$item FAILED"
    failed=yes
    tests_failed=( "${tests_failed[@]}" "$item" )
  else
    echo "$item PASSED"
  fi

done
popd >> /dev/null

echo "------------------------------------------------"
echo "END `date`"

#########################
# Analyse tests outcome #
#########################

if [ $failed = "yes" ]; then

  echo "TEST_FAILED"
  echo "The following tests failed:"
  for item in ${tests_failed[*]}
  do
    echo "$item: results in tests/${item}_result.txt"
  done
else
    echo "TEST_PASSED"
fi

