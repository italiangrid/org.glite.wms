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
# Adapted for DPM: Dimitar Shiyachki <Dimitar.Shiyachki@cern.ch>
#
##############################################################################

showUsage ()
{
 echo "                                           "
 echo "Usage: DPM-certtest.sh                     "
 echo "                                           "
}

exitFailure ()
{
echo "--------------------------------------------------------------"
echo "END `date`"
echo "--------------------------------------------------------------"
echo "OVERALL RESULT: FAILURE"
echo "--------------------------------------------------------------"
exit -1
}

dpm_exec ()
{
   ssh root@"$DPM_HOST" "$1"
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

if [ -e "DPM-certconfig" ]; then
  source ./DPM-certconfig
else
  echo "The file ./DPM-certconfig must be sourced in order to run the tests"
  exitFailure
fi

if [ -z "$CFG_VO" ]; then
  echo "--------------------------------------------------------------"
  echo "You need to set VO in order to run the tests"
  exitFailure
fi

if [ -z "$CFG_LFC_HOST" ]; then
  echo "--------------------------------------------------------------"
  echo "You need to set LFC_HOST in order to run the tests"
  exitFailure
fi

if [ -z "$CFG_DPM_HOST" ]; then
  echo "--------------------------------------------------------------"
  echo "You need to set DPM_HOST in order to run the tests"
  exitFailure
fi

if [ -z "$CFG_DPNS_HOST" ]; then
  echo "--------------------------------------------------------------"
  echo "You need to set DPNS_HOST in order to run the tests"
  exitFailure
fi

export DPM_HOST=$CFG_DPM_HOST
export DPNS_HOST=$CFG_DPNS_HOST
export LFC_HOST=$CFG_LFC_HOST

export CFG_VO=$CFG_VO
if [ -z "$CFG_DPNS_BASEDIR" ]; then
   export CFG_DPNS_BASEDIR="home"
else
   export CFG_DPNS_BASEDIR=$CFG_DPNS_BASEDIR
fi
export CFG_DPNS_DOMAIN=$CFG_DPNS_DOMAIN
export CFG_VO=$CFG_VO

#########
# START #
#########
echo "--------------------------------------------------------------"
echo "START: `date`"
echo
echo "- CLIENT RPMS installed -"
rpm -q -a | grep DPM | sort
rpm -q -a | grep LFC | sort
rpm -q -a | grep lcg-dm-common | sort
echo
echo "- SERVER RPMS installed -"
dpm_exec "rpm -q -a | grep DPM | sort"
dpm_exec "rpm -q -a | grep LFC | sort"
dpm_exec "rpm -q -a | grep lcg-dm-common | sort"
echo
echo "- Other settings -"
echo "VO used: $CFG_VO"
echo "LFC Host: $CFG_LFC_HOST"
echo "DPM Host: $CFG_DPM_HOST"
echo "DPNS Host: $CFG_DPNS_HOST"
echo "--------------------------------------------------------------"

####################################
# Checking if there is valid proxy #
####################################

ProxyExist=`voms-proxy-info 2>/dev/null | grep timeleft | wc -l`

ProxyExpired=`voms-proxy-info 2>/dev/null | grep  "timeleft  : 0:00:00" | wc -l`

if ! [ $ProxyExist -gt 0 -a $ProxyExpired -eq 0 ]; then
  echo "Valid proxy is needed for this test!"
  if [ $ProxyExpired -gt 0 ]; then
    echo "Proxy credential expired!"
  fi
  exitFailure
fi

########################################################################
# Checking if the proxy has at least two group FQANs for the VO in use #
########################################################################

gFQANcount=$(voms-proxy-info -all | grep "^attribute : /$CFG_VO" | wc -l)
if [ $gFQANcount -lt 2 ]; then
  echo "VOMS proxy is needed for this test and it should has at least two group FQANs for the VO used"
  exitFailure
fi

############
# TESTS    #
############
declare -a tests_failed
failed=no

if [ x$CFG_PING_TESTS == "xyes" ]; then

   echo "*Executing Service PING tests"
   testdir=./tests/ping
   pushd $testdir >> /dev/null
   tests_list=(ping-services.sh)

   touch testfile 2> /dev/null
   if [ $? -ne 0 ]; then
      echo "PING test directory is not writable, if you are on AFS be sure to have a valid token"
      exitFailure
   fi

  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt testfile
    ./$item $CFG_DPM_HOST >> ${item}_result.txt 2>&1
    echo
    cat ping-services.sh_result.txt
  done
  popd >> /dev/null
else
 echo "*WARNING: DPM INFOSYS tests skipped"
fi

echo "--------------------------------------------------------------"

if [ x$CFG_DPNS_CLI == "xyes" ]; then

  echo "*Executing DPNS CLI tests"
  testdir=./tests/dpns-cli/

  pushd $testdir >> /dev/null
  tests_list=( CLI-dpns-chgrp  CLI-dpns-chown  CLI-dpns-mkdir CLI-dpns-chmod  CLI-dpns-ls CLI-dpns-ln CLI-dpns-rm CLI-dpns-rename CLI-dpns-getacl CLI-dpns-setacl CLI-dpns-usrmap CLI-dpns-grpmap CLI-dpns-ping )

#  tests_list=( CLI-dpns-getacl )

  touch testfile 2> /dev/null
  if [ $? -ne 0 ]; then
    echo "DPNS CLI test directory is not writable, if you are on AFS be sure to have a valid token"
    exitFailure
  fi
  
  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt testfile
    echo
    echo "Executing $item"
    ./$item >> ${item}_result.txt 2>&1
    res=$?
    if [ "$res" != 0 ]; then
      echo "$item FAILED"
      failed=yes
      tests_failed=( "${tests_failed[@]}" "$item" )
    else
      echo "$item PASSED"
    fi
  done
  popd >> /dev/null

  if [ $failed = "yes" ]; then

     echo "OVERALL RESULT: FAILURE"
     echo
     echo "The following tests have failed:"
     for item in ${tests_failed[*]}
     do
       echo "$item: results in tests/dpns-cli/${item}_result.txt"
     done
   else
     echo "OVERALL RESULT: SUCCESS"
   fi
else
 echo "*WARNING: DPNS CLI tests skipped"
fi

echo "--------------------------------------------------------------"

if [ x$CFG_DPNS_API_C == "xyes" ]; then

   echo "*Executing DPNS-API-C tests"
   testdir=./tests/dpns-api-c
   pushd $testdir >> /dev/null
   tests_list=(dpns_mixed.sh)

   touch testfile 2> /dev/null
   if [ $? -ne 0 ]; then
      echo "DPNS API C test directory is not writable, if you are on AFS be sure to have a valid token"
      exitFailure
   fi

  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt testfile
    echo
    echo "Executing $item"
    ./$item >> ${item}_result.txt 2>&1
    echo
    cat ${item}_result.txt
  done
  popd >> /dev/null
else
 echo "*WARNING: DPNS-API-C tests skipped"
fi

echo "--------------------------------------------------------------"

if [ x$CFG_DPNS_API_PYTHON == "xyes" ]; then

   echo "*Executing DPNS-API-PYTHON tests"
   export TESTSUITE_LOCATION
   testdir=./tests/dpns-api-python
   pushd $testdir >> /dev/null
   tests_list=(dpns_getreplica.sh)
   tests_failed=()

   touch testfile 2> /dev/null
   if [ $? -ne 0 ]; then
      echo "DPNS API PYTHON test directory is not writable, if you are on AFS be sure to have a valid token"
      exitFailure
   fi

  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt testfile
    echo
    echo "Executing $item"
    ./$item >> ${item}_result.txt 2>&1
    res=$?
    if [ "$res" != 0 ]; then
      echo "$item FAILED"
      failed=yes
      tests_failed=( "${tests_failed[@]}" "$item" )
    else
      echo "$item PASSED"
    fi
  done
  popd >> /dev/null
  if [ $failed = "yes" ]; then

     echo "OVERALL RESULT: FAILURE"
     echo
     echo "The following tests have failed:"
     for item in ${tests_failed[*]}
     do
       echo "$item: results in tests/dpns-api-python/${item}_result.txt"
     done
   else
     echo "OVERALL RESULT: SUCCESS"
   fi
else
 echo "*WARNING: DPNS-API-PYTHON tests skipped"
fi


echo "--------------------------------------------------------------"

if [ x$CFG_RFIO_CLI == "xyes" ]; then

   echo "*Executing RFIO tests"
   testdir=./tests/rfio-cli
   pushd $testdir >> /dev/null
   tests_list=(DPM-test-rfio-cli.sh)

   touch testfile 2> /dev/null
   if [ $? -ne 0 ]; then
      echo "RFIO CLI test directory is not writable, if you are on AFS be sure to have a valid token"
      exitFailure
   fi

  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt testfile
    echo
    echo "Executing $item"
    ./$item $CFG_DPM_HOST $CFG_VO >> ${item}_result.txt 2>&1
    echo
    cat ${item}_result.txt
  done
  popd >> /dev/null
else
 echo "*WARNING: RFIO CLI tests skipped"
fi

echo "--------------------------------------------------------------"

if [ x$CFG_DPM_INFOSYS == "xyes" ]; then

   echo "*Executing DPM INFOSYS tests"
   testdir=./tests/infosys
   pushd $testdir >> /dev/null
   tests_list=(test-infosys.sh)

   touch testfile 2> /dev/null
   if [ $? -ne 0 ]; then
      echo "DPM INFOSYS test directory is not writable, if you are on AFS be sure to have a valid token"
      exitFailure
   fi

  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt testfile
    ./$item $CFG_DPM_HOST >> ${item}_result.txt 2>&1
    echo
    cat test-infosys.sh_result.txt
  done
  popd >> /dev/null
else
 echo "*WARNING: DPM INFOSYS tests skipped"
fi


echo
echo "--------------------------------------------------------------"
echo "END: `date`"
echo "--------------------------------------------------------------"


#########################
# Analyse tests outcome #
#########################

if [ $failed = "yes" ]; then

  echo "OVERALL RESULT: FAILURE"
  echo "--------------------------------------------------------------"
else
  echo "OVERALL RESULT: SUCCESS"
  echo "--------------------------------------------------------------"

fi


