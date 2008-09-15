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
 echo "                                           "
 echo "Usage:  AMGA-certtest.sh                   "
 echo "                                           "
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
  
if [ -e "AMGA-certconfig" ]; then
  source ./AMGA-certconfig
else
  echo "The file ./AMGA-certconfig must be sourced in order to run the tests"
  exit -1
fi  

if [ -z "$AMGA_HOST" ]; then
  echo "You need to set AMGA_HOST in AMGA-certconfig in order to run this script"
  exit -1
else
  hostname=$AMGA_HOST
fi

########################
# Launch all the tests #
########################

echo "START `date` "
echo "------------------------------------------------"

testdir=./tests
tests_list=(AMGA-test_ping.py AMGA-test_functions.py AMGA-test_statistics.py)

pushd $testdir >> /dev/null
rm -rf result

declare -a tests_failed
failed=no

for item in ${tests_list[*]}
do
  rm -rf ${item}_result.txt
  echo "Executing $item"
  python $item $hostname >> ${item}_result.txt
  grep '\-TEST FAILED\-' ${item}_result.txt >> /dev/null
  if [ $? -eq 0 ]; then
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



