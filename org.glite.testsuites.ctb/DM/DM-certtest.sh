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
 echo "Usage:  DM-certtest.sh "
 echo "                                           "
}



###################################
# Check for environment variables #
###################################

if [ -e "DM-certconfig" ]; then
  source ./DM-certconfig
else
  echo "The file ./DM-certconfig must be sourced in order to run the tests"
  exit -1
fi

if [ -z "$LFC_HOST" ]; then
  echo "You need to set LFC_HOST in order to run this script"
  exit -1
fi

if [ -z "$LCG_GFAL_INFOSYS" ]; then
  echo "You need to set LCG_GFAL_INFOSYS in order to run this script"
  exit -1
fi

if [ -z "$VO" ]; then
  echo "You need to set LCG_GFAL_INFOSYS in order to run this script"
  exit -1
fi

########################
# Launch all the tests #
########################

echo "START `date` "
echo "------------------------------------------------"

declare -a tests_failed
failed=no

#################
#LCG_UTILS test #
#################

if [ "$LCG_UTILS" = "yes" ]; then

  if [ -z $FIRSTSE ]; then
    echo "LCG_UTILS tests need FIRSTSE to be defined in DM-certconfig"
    exit -1
  fi
  if [ -z $SECONDSE ]; then
    echo "LCG_UTILS tests need SECONDSE to be defined in DM-certconfig"
    exit -1
  fi
  if [ -z $VO ]; then
    echo "LCG_UTILS tests need VO to be defined in DM-certconfig"
    exit -1
  fi

  pushd ./tests >> /dev/null
  tests_list=( DM-lcg-alias.sh DM-lcg-cp-gsiftp.sh DM-lcg-cp.sh DM-lcg-cr-gsiftp.sh DM-lcg-cr.sh DM-lcg-list.sh  DM-lcg-ls.sh DM-lcg-rep.sh DM-lcg-rf.sh )


  echo "*Running LCG_UTILS test set*"
  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt
    echo "Executing $item"
    if [ "$item" = "DM-lcg-alias.sh" -o "$item" = "DM-lcg-cp-gsiftp.sh" -o "$item" = "DM-lcg-cp.sh" \
       -o "$item" = "DM-lcg-cr-gsiftp.sh" -o "$item" = "DM-lcg-cr.sh" -o "$item" = "DM-lcg-list.sh" \
       -o "$item" = "DM-lcg-ls.sh" -o "$item" = "DM-lcg-rf.sh" ]; then
      ./$item $FIRSTSE --vo $VO  >> ${item}_result.txt
    elif [ "$item" = "DM-lcg-rep.sh" ]; then
      ./$item $FIRSTSE $SECONDSE --vo $VO  >> ${item}_result.txt
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
fi

##############
# GFAL tests #
##############

  if [ -z $FIRSTSE ]; then
    echo "GFAL tests need FIRSTSE to be defined in DM-certconfig"
    exit -1
  fi
  if [ -z $VO ]; then
    echo "GFAL tests need VO to be defined in DM-certconfig"
    exit -1
  fi
if [ "$GFAL" = "yes" ]; then
  pushd ../GFAL/tests >> /dev/null
  tests_list=( test-gfal.sh )

  echo "*Running GFAL test set*"
  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt
    echo "Executing $item"
    ./$item -v $VO -l $LFC_HOST -d $FIRSTSE >> ${item}_result.txt 2>&1   
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
fi

#####################
# DM CROSS SE tests #
#####################
if [ "$DM_CROSS_SE" = "yes" ];then
  pushd ../UI/tests >> /dev/null
  tests_list=( test-lcg-utils.sh )
  seoptions=""

  if [ -z $VO ]; then
    echo "DM CROSS SE tests need VO to be defined in DM-certconfig"
    exit -1
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

  echo "*Running DM_CROSS_SE test set*"
  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt
    echo "Executing $item"
    ./$item --vo $VO $seoptions >> ${item}_result.txt 2>&1
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
      echo "$item: results in tests/${item}_result.txt"
    elif [ "$item" = "test-gfal.sh" ]; then
      echo "$item: results in ../GFAL/tests/${item}_result.txt"
    elif [ "$item" = "test-lcg-utils.sh" ]; then
      echo "$item: results in ../UI/tests/${item}_result.txt"
    fi
  done
else 
    echo "TEST_PASSED"
fi



