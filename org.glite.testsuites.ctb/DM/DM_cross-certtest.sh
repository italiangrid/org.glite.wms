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
 echo "Usage:  DM_cross-certtest.sh [-f <conf.file>] [--dpm <DPM HOST>] [--dcache <DCACHE HOST>] [--castor <CASTOR HOST>] [--storm <STORM HOST>]"
 echo "  <conf.file> Configuration file, default is DM-certconfig"
 echo "  <DPM HOST> DPM SE "
 echo "  <DCACHE HOST> dCache SE "
 echo "  <CASTOR HOST> CASTOR SE "
 echo "  <STORM HOST> STORM SE "
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

#Parse arguments
while [ $# -ne 0 ]; do
  case "$1" in
    -f)
      shift
      conffile=$1
      shift
      ;;
    '--dpm')
      shift
      DPMHOST_ARG=$1
      shift
      ;;
    '--dcache')
      shift
      DCACHEHOST_ARG=$1
      shift
      ;;
    '--castor')
      shift
      CASTORHOST_ARG=$1
      shift
      ;;
    '--storm')
      shift
      STORMHOST_ARG=$1
      shift
      ;;
    *|'')
      echo "Unknown option '$1'"
      exit
      ;;
  esac
done



###################################
# Check for environment variables #
###################################

if [ "x$conffile" = "x" ]; then
  #Default value
  conffile="./DM_cross-certconfig"
fi
echo "Using $conffile"

if [ -e $conffile ]; then
  source $conffile
else
  echo "The file $conffile must be sourced in order to run the tests"
  exitFailure
fi

if [ -n "$DPMHOST_ARG" ]; then
  dpm=$DPMHOST_ARG 
elif [ -n "$DPMHOST" ]; then
  dpm=$DPMHOST
else
  echo "DPMHOST not set"
fi

if [ -n "$DCACHEHOST_ARG" ]; then
  dcache=$DCACHEHOST_ARG 
elif [ -n "$DCACHEHOST" ]; then
  dcache=$DCACHEHOST
else
  echo "DCACHEHOST not set"
fi

if [ -n "$CASTORHOST_ARG" ]; then
  castor=$CASTORHOST_ARG 
elif [ -n "$CASTORHOST" ]; then
  castor=$CASTORHOST
else
  echo "CASTORHOST not set"
fi

if [ -n "$STORMHOST_ARG" ]; then
  storm=$STORMHOST_ARG 
elif [ -n "$STORMHOST" ]; then
  storm=$STORMHOST
else
  echo "STORMHOST not set"
fi

if [ -z "$LFC_HOST" ]; then
  echo "You need to set LFC_HOST in order to run this script"
  exitFailure
fi

if [ -z "$LCG_GFAL_INFOSYS" ]; then
  echo "You need to set LCG_GFAL_INFOSYS in order to run this script"
  exitFailure
fi

if [ -z "$VO" ]; then
  echo "You need to set VO in order to run this script"
  exitFailure
fi

#########
# START #
#########

echo "START `date` "
echo "------------------------------------------------"

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

####################################
# Create a directory for log files #
####################################

id=`date +%y%m%d%H%M%S`
if [ -z "$LOGLOCATION" ];then
  cp=`pwd`
  loglocation=$cp/logs_$id
  mkdir -p $loglocation
else
  loglocation=$LOGLOCATION/logs_$id
  mkdir -p $loglocation
fi

if [ ! -d $loglocation ];then
  echo   "Error while creating log directory $loglocation"
  exitFailure
else
  echo "Log files will be stored in $loglocation"
fi

########################
# Launch all the tests #
########################

declare -a tests_failed
failed=no

#####################
# DM CROSS SE tests #
#####################

echo "*Running DM_CROSS_SE test set*"
pushd tests >> /dev/null
tests_list=( test-lcg-utils.sh )
seoptions=""

if [ -z $dpm ]; then
  echo "WARNING: DPM SE will not be tested"
else
  seoptions="$seoptions --dpm $dpm"
fi

if [ -z $dcache ]; then
  echo "WARNING: DCACHE SE will not be tested"
else
  seoptions="$seoptions --dcache $dcache"
fi

if [ -z $castor ]; then
  echo "WARNING: CASTOR SE will not be tested"
else
  seoptions="$seoptions --castor $castor"
fi

if [ -z $storm ]; then
  echo "WARNING: STORM SE will not be tested"
else
  seoptions="$seoptions --storm $castor"
fi

for item in ${tests_list[*]}
do
  rm -rf $loglocation/${item}_result.txt testfile
  echo "Executing $item"
  echo "./$item --vo $VO $seoptions" > $loglocation/${item}_result.txt 2>&1
  ./$item --vo $VO $seoptions >> $loglocation/${item}_result.txt 2>&1
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
    echo "$item: results in $loglocation/${item}_result.txt"
  done
  exit 1
else 
    echo "TEST_PASSED"
  exit 0
fi



