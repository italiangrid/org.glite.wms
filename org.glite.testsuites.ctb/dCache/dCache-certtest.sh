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
 echo "Usage:  $0  [-f <conf.file>] [--dcache <dCache host>]"
 echo "  <conf.file> Configuration file, default is FTS-certconfig"
 echo "  <dCache host> Target SE for the tests, default is \$DCACHE_HOST"
 echo "                                           "
}

exitFailure ()
{
echo "------------------------------------------------"
echo "END `date`"
echo "-TEST FAILED-"
exit -1
}

#Parse arguments
while [ $# -ne 0 ]; do
  case "$1" in
    -f)
      shift
      conffile=$1
      shift
      ;;
    '--dcache')
      shift
      DCACHE_HOST_ARG=$1
      shift
      ;;
    '-h|-help|--help')
      showUsage
      exit 0
      ;;
    *|'')
      echo "Unknown option '$1'"
      exit
      ;;
  esac
done

if [ "x$conffile" = "x" ]; then
  #Default value
  conffile="./dCache-certconfig"
fi
echo "Using $conffile"

if [ -e $conffile ]; then
  source $conffile
else
  echo "The file $conffile must be sourced in order to run the tests"
  exitFailure
fi

if [ -n "$DCACHE_HOST_ARG" ]; then
  hostname=$DCACHE_HOST_ARG
elif [ -n "$DCACHE_HOST" ]; then
  hostname=$DCACHE_HOST
else
  echo "You ned to set DCACHE_HOST in DCACHE-certconfig or use the --dcache argument"
  exitFailure
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
echo "DCACHE HOST: $hostname"

############################
# check valid proxy #
############################

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

declare -a tests_list
declare -a tests_failed
failed=no

RHVER=`cat /etc/redhat-release |  awk -F'release ' '{print $2}' | cut -c 1`

#COMMON TESTS
tests_list=("${tests_list[@]}" "test_srmMkdir_passive")
tests_list=("${tests_list[@]}" "test_srm_passive")

#ADD TESTS if SL4
if [ "x$RHVER" == "x4" ]; then
  tests_list=("${tests_list[@]}" "test_GlobusUrlCopyGridftp")
  tests_list=("${tests_list[@]}" "test_edgGridftpLsRoot")
  tests_list=("${tests_list[@]}" "test_GlobusUrlCopyStreams")
fi

for item in ${tests_list[*]}
do
  rm -rf $loglocation/${item}_result.txt testfile
  echo "Executing $item"
  echo "dCacheTestSuite.py -T $hostname -d desy.de -v dteam -q 0 -r $item" > $loglocation/${item}_result.txt
  dCacheTestSuite.py -T $hostname -d desy.de -v dteam -q 0 -r $item &> > $loglocation/${item}_result.txt
  if [ $? -ne 0 ]; then
    echo "$item FAILED"
    failed=yes
    tests_failed=( "${tests_failed[@]}" "$item" )
  else
    echo "$item PASSED"
  fi
done

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


