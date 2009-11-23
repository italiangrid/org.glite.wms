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
 echo "Usage:  FTS-certtest.sh  [-f <conf.file>] [--fts <FTS HOST>]"
 echo "  <conf.file> Configuration file, default is FTS-certconfig"
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
if [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$1" = "--help" ]; then
  showUsage
  exit 2
fi

###################################
# Check for environment variables #
###################################

#Parse arguments
while [ $# -ne 0 ]; do
  case "$1" in
    -f)
      shift
      conffile=$1
      shift
      ;;
    '--fts')
      shift
      FTS_HOST_ARG=$1
      shift
      ;;
    *|'')
      echo "Unknown option '$1'"
      exit
      ;;
  esac
done


if [ "x$conffile" = "x" ]; then
  #Default value
  conffile="./FTS-certconfig"
fi
echo "Using $conffile"

if [ -e $conffile ]; then
  source $conffile
else
  echo "The file $conffile must be sourced in order to run the tests"
  exitFailure
fi

if [ -n "$FTS_HOST_ARG" ]; then
  hostname=$FTS_HOST_ARG
elif [ -n "$FTS_HOST" ]; then
  hostname=$FTS_HOST
else
  echo "You ned to set FTS_HOST in FTS-certconfig or use the --fts argument"
  exitFailure
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

if [ -z "$CHANNELS" ]; then
  echo "You need to set CHANNELS in FTS-certconfig in order to run the tests"
  exitFailure
else
  channels=$CHANNELS
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

####################################
# Create a directory for log files #
####################################

id=`date +%y%m%d%H%M%S`
if [ -z "$LOGSLOCATION" ]; then
  cp=`pwd`
  loglocation=$cp/logs_$id
  mkdir -p $loglocation
else
  loglocation=$LOGSLOCATION/logs_$id
  mkdir -p $loglocation
fi

if [ ! -d $loglocation ];then
  echo   "Error while creating log directory $loglocation"
  exitFailure
else
  echo "Log files will be stored in $loglocation"
fi

#########
# START #
#########

echo "START `date` "
echo "------------------------------------------------"
echo "FTS HOST: $FTS_HOST"

############################
# Generate and check proxy #
############################


echo "test" | voms-proxy-init --voms $VO -pwstdin 2>1 >> /dev/null
if [ $? -ne 0 ]; then
  echo "Failed creating a proxy"
  exitFailure
fi

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

#########
# BASIC #
#########

if [ "x${BASIC}" = "xyes" ]; then
  echo "*Running BASIC tests"
  testdir=./tests
  declare -a tests_list
  tests_list=("${tests_list[@]}" "FTS-basic")
  tests_list=("${tests_list[@]}" "FTS-services")
  tests_list=("${tests_list[@]}" "FTS-channels")
  tests_list=("${tests_list[@]}" "FTS-submission")
  pushd $testdir >> /dev/null
  touch testfile 2> /dev/null
  if [ $? -ne 0 ]; then
    echo "FTS test directory is not writable, if you are on AFS be sure to have a valid token"
    exitFailure
  fi

  for item in ${tests_list[*]}
  do
    rm -rf $loglocation/${item}_result.txt testfile
    echo "Executing $item"
    if [ $item = "FTS-basic" ]; then
      echo "./$item --fts $hostname --bdii $bdiihost" > $loglocation/${item}_result.txt
      ./$item --fts $hostname --bdii $bdiihost >> $loglocation/${item}_result.txt
      res=$?
    elif [ $item = "FTS-services" ]; then
      echo "./$item --fts $hostname --bdii $bdiihost --site $sitename" \
          > $loglocation/${item}_result.txt
      ./$item --fts $hostname --bdii $bdiihost --site $sitename \
                                                  >> $loglocation/${item}_result.txt
      res=$?
    elif [ $item = "FTS-basic" ]; then
      echo "./$item --fts $hostname --bdii $bdiihost" > $loglocation/${item}_result.txt
      ./$item --fts $hostname --bdii $bdiihost >> $loglocation/${item}_result.txt
      res=$?
    elif [ $item = "FTS-channels" ]; then
      echo "./$item --fts $hostname" > $loglocation/${item}_result.txt
      ./$item --fts $hostname >> $loglocation/${item}_result.txt
      res=$?
    elif [ $item = "FTS-submission" ]; then
      for channel in $channels
      do
        echo "./$item --fts $hostname --bdii $bdiihost --channel $channel --vo $voname --timeout $time" >> $loglocation/${item}_result.txt
        ./$item --fts $hostname --bdii $bdiihost --channel $channel \
              --vo $voname --timeout $time >> $loglocation/${item}_result.txt
        res=$?
      done
    fi
    grep '\-TEST FAILED\-' $loglocation/${item}_result.txt >> /dev/null
    if [ "$?" = 0 -o "$res" != 0 ]; then
      echo "$item FAILED"
      failed=yes
      tests_failed=( "${tests_failed[@]}" "$item" )
    else
      echo "$item PASSED"
    fi
  done
  popd >>/dev/null
else
  echo "*BASIC tests skipped"
fi
############################
# CHECKSUM (FROM FTS 2.2.1)#
############################
unset tests_list

if [ "x${CHECKSUM}" = "xyes" ]; then
  echo "*Running CHECKSUM tests"
  testdir=./tests
  declare -a tests_list
  tests_list=("${tests_list[@]}" "FTS-submission-with-checksum-1.sh")
  tests_list=("${tests_list[@]}" "FTS-submission-with-checksum-2.sh")
  tests_list=("${tests_list[@]}" "FTS-submission-with-checksum-3.sh")
  pushd $testdir >> /dev/null
  touch testfile 2> /dev/null
  for item in ${tests_list[*]}
  do
    rm -rf $loglocation/${item}_result.txt
    echo "Executing $item"
    echo "./$item --fts $hostname --bdii $bdiihost --channel $channel \
            --vo $voname --timeout $time" > $loglocation/${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost --channel $channel \
            --vo $voname --timeout $time >> $loglocation/${item}_result.txt
    res=$?
    grep '\-TEST FAILED\-' $loglocation/${item}_result.txt >> /dev/null
    if [ "$?" = 0 -o "$res" != 0 ]; then
      echo "$item FAILED"
      failed=yes
      tests_failed=( "${tests_failed[@]}" "$item" )
    else
      echo "$item PASSED"
    fi
  done
  popd >>/dev/null
else
  echo "*CHECKSUM tests skipped"
fi

#########
# SPACE #
#########
unset tests_list

if [ "x${SPACE}" = "xyes" ]; then
  if [ "x$CERN_SE" != "x" ]; then
    SOURCESE=$CERN_SE
  else
    echo "CERN_SE has to be set to run SPACE tests"
  fi
  if [ "x$CERN_SE2" != "x" ]; then
    DESTSE=$CERN_SE2
  else
    echo "CERN_SE2 has to be set to run SPACE tests"
  fi

  echo "*Running SPACE tests"
  testdir=./tests
  declare -a tests_list
  tests_list=("${tests_list[@]}" "FTS-srmspace")
  tests_list=("${tests_list[@]}" "FTS-srmspace2")
  pushd $testdir >> /dev/null
  touch testfile 2> /dev/null
  for item in ${tests_list[*]}
  do
    rm -rf $loglocation/${item}_result.txt
    echo "Executing $item"
    echo "./$item --fts $hostname --bdii $bdiihost --src $SOURCESE --dest $DESTSE --vo $voname --timeout $time" > $loglocation/${item}_result.txt
    ./$item --fts $hostname --bdii $bdiihost --src $SOURCESE --dest $DESTSE --vo $voname --timeout $time >> $loglocation/${item}_result.txt
    res=$?
    grep '\-TEST FAILED\-' $loglocation/${item}_result.txt >> /dev/null
    if [ "$?" = 0 -o "$res" != 0 ]; then
      echo "$item FAILED"
      failed=yes
      tests_failed=( "${tests_failed[@]}" "$item" )
    else
      echo "$item PASSED"
    fi
  done
  popd >>/dev/null
else
  echo "*SPACE tests skipped"
fi

#######
# END #
#######
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
    echo "$item: results in $loglocation/${item}_result.txt"
  done
  exit 1
else
  echo "TEST_PASSED"
  exit 0
fi

