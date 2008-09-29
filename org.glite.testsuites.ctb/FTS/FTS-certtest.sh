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
  exit -1
fi

if [ -z "$FTS_HOST" ]; then
  echo "You need to set FTS_HOST in FTS-certconfig in order to run this script"
  exit -1
else
  hostname=$FTS_HOST
fi

if [ -z "$BDII_HOST" ]; then
  echo "You need to set BDII_HOST in FTS-certconfig in order to run this script"
  exit -1
else
  bdiihost=$BDII_HOST
fi

if [ -z "$LFC_HOST" ]; then
  echo "You need to set LFC_HOST in FTS-certconfig in order to run this script"
  exit -1
else
  bdiihost=$BDII_HOST
fi

if [ -z "$SITE" ]; then
  echo "You need to set SITE in FTS-certconfig in order to run this script"
  exit -1
else
  sitename=$SITE
fi

if [ -z "$CHANNEL" ]; then
  echo "You need to set CHANNEL in FTS-certconfig in order to run this script"
  exit -1
else
  channel=$CHANNEL
fi

if [ -z "$VO" ]; then
  echo "You need to set VO in FTS-certconfig in order to run this script"
  exit -1
else
  voname=$VO
fi

if [ -z "$TIMEOUT" ]; then
  echo "You need to set TIMEOUT in FTS-certconfig in order to run this script"
  exit -1
else
  time=$TIMEOUT
fi

if [ -z "$NJOBS" ]; then
  echo "You need to set NJOBS in FTS-certconfig in order to run this script"
  exit -1
else
  njobs=$NJOBS
fi

if [ -z "$NFILES" ]; then
  echo "You need to set NFILES in FTS-certconfig in order to run this script"
  exit -1
else
  nfiles=$NFILES
fi


########################
# Launch all the tests #
########################

echo "START `date` "
echo "------------------------------------------------"

testdir=./tests
tests_list=(FTS-basic FTS-services FTS-channels FTS-submission)
#tests_list=(FTS-basic FTS-services FTS-channels FTS-submission FTS-stress FTS-all-channels)

pushd $testdir >> /dev/null
rm -rf result

declare -a tests_failed
failed=no

for item in ${tests_list[*]}
do
  rm -rf ${item}_result.txt
  echo "Executing $item"
  if [ $item = "FTS-basic" ]; then
    echo "./$item --fts $hostname --bdii $bdiihost" > ${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost >> ${item}_result.txt
  fi
  if [ $item = "FTS-services" ]; then
    echo "./$item --fts $hostname --bdii $bdiihost --site $sitename" \
        > ${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost --site $sitename \
                                                >> ${item}_result.txt
  fi
  if [ $item = "FTS-basic" ]; then
    echo "./$item --fts $hostname --bdii $bdiihost" > ${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost >> ${item}_result.txt
  fi
  if [ $item = "FTS-channels" ]; then
    echo "./$item --fts $hostname" > ${item}_result.txt
    ./$item --fts $hostname >> ${item}_result.txt
  fi
  if [ $item = "FTS-submission" ]; then
    echo "./$item --fts $hostname --bdii $bdiihost --channel $channel \
            --vo $voname --timeout $time" > ${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost --channel $channel \
            --vo $voname --timeout $time >> ${item}_result.txt
  fi

  if [ $item = "FTS-all-channels" ]; then
    echo "./$item --fts $hostname --bdii $bdiihost --site $sitename \
            --vo $voname --timeout $time " > ${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost --site $sitename \
            --vo $voname --timeout $time >> ${item}_result.txt
  fi

   if [ $item = "FTS-stress" ]; then
    echo "./$item --fts $hostname --bdii $bdiihost --site $sitename \
            --vo $voname --njobs $njobs  --nfiles $nfiles" > ${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost --site $sitename \
            --vo $voname --njobs $njobs  --nfiles $nfiles >> ${item}_result.txt
  fi

 
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

