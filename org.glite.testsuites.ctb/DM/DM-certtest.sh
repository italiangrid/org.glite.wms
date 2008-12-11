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
 echo "Usage:  DM-certtest.sh [-f <conf.file>]    "
 echo "  <conf.file> Configuration file, default is DM-certconfig"
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

if [ "$1" = "-f" ]; then
  conffile=$2
else
  conffile="./DM-certconfig"
fi
###################################
# Check for environment variables #
###################################

if [ -e $conffile ]; then
  echo "Using $conffile"
  source $conffile
else
  echo "Config File $conffile does not exist"
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
  echo "You need to set LCG_GFAL_INFOSYS in order to run this script"
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

if [ -z "$LOGSLOCATION" ];then
  cp=`pwd`
  id=`date +%y%m%d%H%M%S`
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

#################
#LCG_UTILS test #
#################

if [ "$LCG_UTILS" = "yes" ]; then
  echo "*Running LCG_UTILS test set*"

  if [ -z $FIRSTSE ]; then
    echo "LCG_UTILS tests need FIRSTSE to be defined in DM-certconfig"
    exitFailure
  fi
  if [ -z $SECONDSE ]; then
    echo "LCG_UTILS tests need SECONDSE to be defined in DM-certconfig"
    exitFailure
  fi
  if [ -z $VO ]; then
    echo "LCG_UTILS tests need VO to be defined in DM-certconfig"
    exitFailure
  fi

  touch ../GFAL/tests/testfile 2> /dev/null 
  if [ $? -ne 0 ]; then
    echo "GFAL test directory is not writable, if you are on AFS be sure to have a valid token"
    exitFailure
  fi
  pushd ./tests >> /dev/null
  tests_list=( DM-lcg-alias.sh DM-lcg-cp-gsiftp.sh DM-lcg-cp.sh DM-lcg-cr-gsiftp.sh DM-lcg-cr.sh DM-lcg-list.sh  DM-lcg-ls.sh DM-lcg-rep.sh DM-lcg-rf.sh )


  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt testfile
    echo "Executing $item"
    if [ "$item" = "DM-lcg-alias.sh" -o "$item" = "DM-lcg-cp-gsiftp.sh" -o "$item" = "DM-lcg-cp.sh" \
       -o "$item" = "DM-lcg-cr-gsiftp.sh" -o "$item" = "DM-lcg-cr.sh" -o "$item" = "DM-lcg-list.sh" \
       -o "$item" = "DM-lcg-ls.sh" -o "$item" = "DM-lcg-rf.sh" ]; then
      ./$item $FIRSTSE --vo $VO  > $loglocation/${item}_result.txt
      res=$?
    elif [ "$item" = "DM-lcg-rep.sh" ]; then
      ./$item $FIRSTSE $SECONDSE --vo $VO  > $loglocation/${item}_result.txt
      res=$?
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
  popd >> /dev/null
else
  echo "*LCG_UTILS tests skipped"
fi

##############
# GFAL tests #
##############
if [ "$GFAL" = "yes" ]; then
  echo "*Running GFAL test set*"

  if [ -z $FIRSTSE ]; then
    echo "GFAL tests need FIRSTSE to be defined in DM-certconfig"
    exitFailure
  fi
  if [ -z $VO ]; then
    echo "GFAL tests need VO to be defined in DM-certconfig"
    exitFailure
  fi

  if [ ! -d ../GFAL/tests ]; then
    echo "GFAL test directory does not exists, check it out from CVS!"
    exitFailure
  fi
  touch ../GFAL/tests/testfile 2> /dev/null 
  if [ $? -ne 0 ]; then
    echo "GFAL test directory is not writable, if you are on AFS be sure to have a valid token"
    exitFailure
  fi
  pushd ../GFAL/tests >> /dev/null
  
  tests_list=( test-gfal.sh )

  for item in ${tests_list[*]}
  do
    rm -rf $loglocation/${item}_result.txt testfile
    echo "Executing $item"
    ./$item -v $VO -l $LFC_HOST -d $FIRSTSE > $loglocation/${item}_result.txt 2>&1
    res=$?
    grep '\-TEST FAILED\-' $loglocation/${item}_result.txt > /dev/null
    if [ "$?" = 0 -o "$res" != 0 ]; then
      echo "$item FAILED"
      failed=yes
      tests_failed=( "${tests_failed[@]}" "$item" )
    else
      echo "WARNING! lcg-gt lcg-sd have been skipped due to bug #43002"
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
  if [ ! -d ../UI/tests ]; then
    echo "UI test directory does not exists, check it out from CVS!"
    exitFailure
  fi

  touch ../GFAL/tests/testfile 2> /dev/null 
  if [ $? -ne 0 ]; then
    echo "GFAL test directory is not writable, if you are on AFS be sure to have a valid token"
    exitFailure
  fi
  pushd ../UI/tests >> /dev/null
  tests_list=( test-lcg-utils.sh )
  seoptions=""

  if [ -z $VO ]; then
    echo "DM CROSS SE tests need VO to be defined in DM-certconfig"
    exitFailure
  fi

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



