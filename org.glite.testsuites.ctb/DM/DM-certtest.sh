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
cat <<EOF
Usage:  DM-certtest.sh [-f <conf.file>] [--dpm <DPM HOST>] [--dcache <DCACHE HOST] [--lfc <LFC HOST>]
  <conf.file> Configuration file, default is DM-certconfig
  <DPM HOST> = specify a DPM SE, defaults to the CTB one
  <DCACHE HOST> = specify a DCACHE SE, defaults to the CTB one
  <LFC HOST> = specify an LFC host, defaults to the CTB one
EOF
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
      DPM_HOST_ARG=$1
      shift
      ;;
    '--dcache')
      shift
      DCACHE_HOST_ARG=$1
      shift
      ;;
    '--lfc')
      shift
      LFC_HOST_ARG=$1
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

SEs=""
if [ -n "$DPM_HOST_ARG" ]; then
  SEs=$DPM_HOST_ARG
elif [ -n "$DPM_HOST" ]; then
  SEs=$DPM_HOST
else
  echo "WARNING: no DPM host selected"
fi

if [ -n "$DCACHE_HOST_ARG" ]; then
  SEs="$SEs $DCACHE_HOST_ARG"
elif [ -n "$DCACHE_HOST" ]; then
  SEs="$SEs $DCACHE_HOST"
else
  echo "WARNING: no dCache host selected"
fi

if [ -n "$CASTOR_HOST_ARG" ]; then
  SEs="$SEs $CASTOR_HOST_ARG"
elif [ -n "$CASTOR_HOST" ]; then
  SEs="$SEs $CASTOR_HOST"
else
  echo "WARNING: no CASTOR host selected"
fi

if [ -n "$STORM_HOST_ARG" ]; then
  SEs="$SEs $STORM_HOST_ARG"
elif [ -n "$STORM_HOST" ]; then
  SEs="$SEs $STORM_HOST"
else
  echo "WARNING: no STORM host selected"
fi

if [ "x$SEs" == "x" ]; then
  echo "ERROR: no SEs have been selected"
  exitFailure
fi

if [ -n "$LFC_HOST_ARG" ]; then
  lfc=$LFC_HOST_ARG
elif [ -n "$LFC_HOST" ]; then
  lfc=$LFC_HOST
else
  echo "WARNING: an LFC host has to be specified either with LFC_HOST or --lfc"
  exitFailure
fi
export LFC_HOST=$lfc
echo "LFC host is: $LFC_HOST"

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

#################
#LCG_UTILS test #
#################

if [ "$LCG_UTILS" = "yes" ]; then
  echo "**Running LCG_UTILS tests**"

  pushd ./tests >> /dev/null
  touch testfile 2> /dev/null 
  if [ $? -ne 0 ]; then
    echo "DM tests directory is not writable, if you are on AFS be sure to have a valid token"
    exitFailure
  fi
#  tests_list=( DM-lcg-alias.sh DM-lcg-cp-gsiftp.sh DM-lcg-cp.sh DM-lcg-cr-gsiftp.sh DM-lcg-cr.sh DM-lcg-list.sh  DM-lcg-ls.sh DM-lcg-rep.sh DM-lcg-rf.sh )
  tests_list=( DM-lcg-alias.sh DM-lcg-cp-gsiftp.sh DM-lcg-cp.sh DM-lcg-cr-gsiftp.sh DM-lcg-cr.sh DM-lcg-list.sh  DM-lcg-ls.sh DM-lcg-rf.sh DM-lcg-rep.sh DM-lcg-get-checksum.sh )

  for sehost in $SEs
  do
    echo "*Target SE is $sehost"
    for item in ${tests_list[*]}
    do
      rm -rf ${item}_result.txt testfile
      echo "Executing $item"
      if [ $item == "DM-lcg-rep.sh" ] || [ $item == "DM-lcg-get-checksum.sh" ]; then
        if [ ${#SEs[@]} -gt 1 ]; then
          ./$item ${SEs[0]} ${SEs[1]} --vo $VO  > $loglocation/${item}_result.txt
        else
          echo "WARNING:At least two SEs are needed to run $item, test skipped"
        fi
      else
        ./$item $sehost --vo $VO  > $loglocation/${item}_result.txt
      fi
      res=$?
      grep '\-TEST FAILED\-' $loglocation/${item}_result.txt >> /dev/null
      if [ "$?" = 0 -o "$res" != 0 ]; then
        echo "$item FAILED on $sehost"
        failed=yes
        tests_failed=( "${tests_failed[@]}" "$item" )
      else 
        echo "$item PASSED"
      fi
    done
  done
  popd >> /dev/null
else
  echo "**LCG_UTILS tests skipped"
fi

#############
# SAM tests #
#############
if [ "$SAME" = "yes" ]; then
  echo "**Running SAME tests**"

  pushd ./tests/SAME/tests >> /dev/null
  touch testfile 2> /dev/null
  if [ $? -ne 0 ]; then
    echo "SAME tests direcotry is not writable, if you are on AFS be sure to have a valid token"
    exitFailure
  fi
  tests_list=( SE-lcg-cr  SE-lcg-cp  SE-lcg-del )

  for sehost in $SEs
  do
    echo "*Target SE is $sehost"
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
    echo "$item: results in $loglocation/${item}_result.txt"
  done
  exit 1
else 
    echo "TEST_PASSED"
  exit 0
fi

