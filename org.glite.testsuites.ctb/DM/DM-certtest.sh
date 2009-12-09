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
 echo "Usage:  DM-certtest.sh [-f <conf.file>] [--sehost <SE HOST>] "
 echo "  <conf.file> Configuration file, default is DM-certconfig"
 echo "  --sehost The SE used for all the commands"
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
if [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$1" = "--help" ] || [ $# -gt 2 ]; then
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
    '--sehost')
      shift
      SE_HOST_ARG=$1
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
  conffile="./DM-certconfig"
fi
echo "Using $conffile"

if [ -e $conffile ]; then
  source $conffile
else
  echo "The file $conffile must be sourced in order to run the tests"
  exitFailure
fi

if [ -n "$SE_HOST_ARG" ]; then
  sehost=$SE_HOST_ARG 
elif [ -n "$SE_HOST" ]; then
  sehost=$SE_HOST
else
  echo "You ned to set SE_HOST in DM-certconfig or use the --sehost argument"
  exitFailure
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

echo "SE: $sehost"

#################
#LCG_UTILS test #
#################

if [ "$LCG_UTILS" = "yes" ]; then
  echo "*Running LCG_UTILS test set*"

  pushd ./tests >> /dev/null
  touch testfile 2> /dev/null 
  if [ $? -ne 0 ]; then
    echo "DM tests directory is not writable, if you are on AFS be sure to have a valid token"
    exitFailure
  fi
#  tests_list=( DM-lcg-alias.sh DM-lcg-cp-gsiftp.sh DM-lcg-cp.sh DM-lcg-cr-gsiftp.sh DM-lcg-cr.sh DM-lcg-list.sh  DM-lcg-ls.sh DM-lcg-rep.sh DM-lcg-rf.sh )
  tests_list=( DM-lcg-alias.sh DM-lcg-cp-gsiftp.sh DM-lcg-cp.sh DM-lcg-cr-gsiftp.sh DM-lcg-cr.sh DM-lcg-list.sh  DM-lcg-ls.sh DM-lcg-rf.sh )


  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt testfile
    echo "Executing $item"
    ./$item $sehost --vo $VO  > $loglocation/${item}_result.txt
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
else
  echo "*LCG_UTILS tests skipped"
fi

##############
# GFAL tests #
##############
if [ "$GFAL" = "yes" ]; then
  echo "*Running GFAL test set*"

  if [ ! -d ../GFAL/tests ]; then
    echo "GFAL test directory does not exists, check it out from CVS!"
    exitFailure
  fi

  pushd ../GFAL/tests >> /dev/null
  touch testfile 2> /dev/null
  if [ $? -ne 0 ]; then
    echo "GFAL test directory is not writable, if you are on AFS be sure to have a valid token"
    exitFailure
  fi
  
  tests_list=( test-gfal.sh )

  for item in ${tests_list[*]}
  do
    rm -rf $loglocation/${item}_result.txt testfile
    echo "Executing $item"
    echo "./$item -v $VO -l $LFC_HOST -d $sehost " > $loglocation/${item}_result.txt 2>&1
    ./$item -v $VO -l $LFC_HOST -d $sehost >> $loglocation/${item}_result.txt 2>&1
    res=$?
    grep '\-TEST FAILED\-' $loglocation/${item}_result.txt > /dev/null
    if [ "$?" = 0 -o "$res" != 0 ]; then
      echo "$item FAILED"
      failed=yes
      tests_failed=( "${tests_failed[@]}" "$item" )
    else
      echo "$item PASSED"
    fi
  done
  popd >> /dev/null
else
  echo "*GFAL tests skipped"
fi

#####################
# DM CROSS SE tests #
#####################

if [ "$DM_CROSS_SE" = "yes" ];then
  echo "*Running DM_CROSS_SE test set*"
  pushd tests >> /dev/null
  tests_list=( test-lcg-utils.sh )
  seoptions=""

  if [ -z $CLASSICHOST ]; then
    echo "WARNING: CLASSIC SE will not be tested"
  else
    seoptions="$seoptions --classic $CLASSICHOST"
  fi

  if [ -z $DPMHOST ]; then
    echo "WARNING: DPM SE will not be tested"
  else
    seoptions="$seoptions --dpm $DPMHOST"
  fi

  if [ -z $DCACHEHOST ]; then
    echo "WARNING: DCACHE SE will not be tested"
  else
    seoptions="$seoptions --dcache $DCACHEHOST"
  fi

  if [ -z $CASTORHOST ]; then
    echo "WARNING: CASTOR SE will not be tested"
  else
    seoptions="$seoptions --castor $CASTORHOST"
  fi

  for item in ${tests_list[*]}
  do
    rm -rf $loglocation/${item}_result.txt testfile
    echo "Executing $item"
    ./$item --vo $VO $seoptions > $loglocation/${item}_result.txt 2>&1
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
else
  echo "*DM_CROSS_SE tests skipped"
fi

echo "------------------------------------------------"
echo "END `date`"

#############
# SAM tests #
#############
if [ "$SAME" = "yes" ]; then
  echo "*Running SAME test set*"

  pushd ./tests/SAME/tests >> /dev/null
  touch testfile 2> /dev/null
  if [ $? -ne 0 ]; then
    echo "SAME tests direcotry is not writable, if you are on AFS be sure to have a valid token"
    exitFailure
  fi
  tests_list=( SE-lcg-cr  SE-lcg-cp  SE-lcg-del )

  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt testfile
    echo "Executing $item"
    ./$item $sehost > $loglocation/${item}_result.txt 2>&1
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
else 
  echo "*SAME tests skipped" 
fi  

#########################
# Analyse tests outcome #
#########################

if [ $failed = "yes" ]; then

  echo "TEST_FAILED"
  echo "The following tests failed:"
  for item in ${tests_failed[*]}
  do
    if echo $item | grep DM-lcg.*.sh; then
      echo "$item: results in $loglocation/${item}_result.txt"
    elif [ "$item" = "test-gfal.sh" ]; then
      echo "$item: results in $loglocation/${item}_result.txt"
    elif [ "$item" = "test-lcg-utils.sh" ]; then
      echo "$item: results in $loglocation/${item}_result.txt"
    fi
  done
  exit 1
else 
    echo "TEST_PASSED"
  exit 0
fi



