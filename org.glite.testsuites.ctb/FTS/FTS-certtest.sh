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
 echo "Usage:  FTS-certtest.sh                   "
 echo "                                           "
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

if [ -e "FTS-certconfig" ]; then
  source ./FTS-certconfig
else
  echo "The file ./FTS-certconfig must be sourced in order to run the tests"
  exitFailure
fi

if [ -z "$FTS_HOST" ]; then
  echo "You ned to set FTS_HOST in FTS-certconfig in order to run the tests"
  exitFailure
else
  hostname=$FTS_HOST
fi

if [ -z "$BDII_HOST" ]; then
  echo "You need to set BDII_HOST in FTS-certconfig in order to run the tests"
  exitFailure
else
  bdiihost=$BDII_HOST
fi

if [ -z "$LFC_HOST" ]; then
  echo "You need to set LFC_HOST in FTS-certconfig in order to run the tests"
  exitFailure
else
  bdiihost=$BDII_HOST
fi

if [ -z "$SITE_NAME" ]; then
  echo "You need to set SITE_NAME in FTS-certconfig in order to run the tests"
  exitFailure
else
  sitename=$SITE_NAME
fi

if [ -z "$CHANNEL" ]; then
  echo "You need to set CHANNEL in FTS-certconfig in order to run the tests"
  exitFailure
else
  channel=$CHANNEL
fi

if [ -z "$VO" ]; then
  echo "You need to set VO in FTS-certconfig in order to run the tests"
  exitFailure
else
  voname=$VO
fi

if [ -z "$TIMEOUT" ]; then
  echo "You need to set TIMEOUT in FTS-certconfig in order to run the tests"
  exitFailure
else
  time=$TIMEOUT
fi

if [ -z "$NJOBS" ]; then
  echo "You need to set NJOBS in FTS-certconfig in order to run the tests"
  exitFailure
else
  njobs=$NJOBS
fi

if [ -z "$NFILES" ]; then
  echo "You need to set NFILES in FTS-certconfig in order to run the tests"
  exitFailure
else
  nfiles=$NFILES
fi


#########
# START #
#########

echo "START `date` "
echo "------------------------------------------------"
echo "FTS HOST: $FTS_HOST"

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
tests_list=(FTS-basic FTS-services FTS-channels FTS-submission FTS-submission-with-checksum-1.sh FTS-submission-with-checksum-2.sh FTS-submission-with-checksum-3.sh)
#The last two are failing on CERN-DESY channels
#tests_list=(FTS-basic FTS-services FTS-channels FTS-submission FTS-stress FTS-channels-submit)

pushd $testdir >> /dev/null
touch testfile 2> /dev/null
if [ $? -ne 0 ]; then
  echo "FTS test directory is not writable, if you are on AFS be sure to have a valid token"
  exitFailure
fi

for item in ${tests_list[*]}
do
  rm -rf ${item}_result.txt testfile
  echo "Executing $item"
  if [ $item = "FTS-basic" ]; then
    echo "./$item --fts $hostname --bdii $bdiihost" > ${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost >> ${item}_result.txt
    res=$?
  elif [ $item = "FTS-services" ]; then
    echo "./$item --fts $hostname --bdii $bdiihost --site $sitename" \
        > ${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost --site $sitename \
                                                >> ${item}_result.txt
    res=$?
  elif [ $item = "FTS-basic" ]; then
    echo "./$item --fts $hostname --bdii $bdiihost" > ${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost >> ${item}_result.txt
    res=$?
  elif [ $item = "FTS-channels" ]; then
    echo "./$item --fts $hostname" > ${item}_result.txt
    ./$item --fts $hostname >> ${item}_result.txt
    res=$?
  elif [ $item = "FTS-submission" ]; then
    echo "./$item --fts $hostname --bdii $bdiihost --channel $channel \
            --vo $voname --timeout $time" > ${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost --channel $channel \
            --vo $voname --timeout $time >> ${item}_result.txt
    res=$?
  elif [ $item = "FTS-all-channels" ]; then
    echo "./$item --fts $hostname --bdii $bdiihost --site $sitename \
            --vo $voname --timeout $time " > ${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost --site $sitename \
            --vo $voname --timeout $time >> ${item}_result.txt
    res=$?
  elif [ $item = "FTS-stress" ]; then
    echo "./$item --fts $hostname --bdii $bdiihost --site $sitename \
            --vo $voname --njobs $njobs  --nfiles $nfiles" > ${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost --site $sitename \
            --vo $voname --njobs $njobs  --nfiles $nfiles >> ${item}_result.txt
    res=$?
  elif [ $item = "FTS-submission-with-checksum-1.sh" ] || [ $item = "FTS-submission-with-checksum-2.sh" ] || [ $item = "FTS-submission-with-checksum-3.sh" ]; then
    echo "./$item --fts $hostname --bdii $bdiihost --channel $channel \
            --vo $voname --timeout $time" > ${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost --channel $channel \
            --vo $voname --timeout $time >> ${item}_result.txt
    res=$?
  fi

  grep '\-TEST FAILED\-' ${item}_result.txt >> /dev/null
  if [ "$?" = 0 -o "$res" != 0 ]; then
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

