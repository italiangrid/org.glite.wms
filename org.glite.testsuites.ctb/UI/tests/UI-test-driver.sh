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
# AUTHORS: Andreas Unterkircher, CERN
#
##############################################################################

usage() {
 echo
 echo "Test driver for executing several UI tests in one go. You can run the tests: UI-data-lcg-*, UI-data-lfc-*, UI-inf-lcg-* (one or several)"
 echo "Usage:"
 echo "======"
 echo "UI-test-driver.sh --sehost|-sehost <SE hostname> --lfchost|-lfchost <LFC hostname> --lfcdir|-lfcdir <LFC directory> --vo|-vo <VO name> UI-data-lcg|UI-data-lfc|UI-inf-lcg"
 echo "UI-data-lcg options: --sehost (mandatory), --vo (optional)"
 echo "UI-data-lfc options: --lfchost (mandatory if LFC_HOST is not defined in your environment), --lfcdir (optional)"
 echo "UI-inf-lcg-info options: --vo (optional)"
 echo
}

record_result() {
 if [ $testResult -eq 0 ]; then
  totalNameTestOK[${totalNrTestOK}]=$testName
  ((totalNrTestOK++))
 else
  totalNameTestFAIL[${totalNrTestFAIL}]=$testName
  ((totalNrTestFAIL++))
 fi
}

total_test_result() {
 echo
 echo "============================================="

 echo "${totalNrTest} tests executed"
 echo "${totalNrTestFAIL} tests FAILED"
 echo "${totalNrTestOK} tests are OK"
 echo
 echo "Tests that are OK:"
 i=0
 while [ $i -lt ${totalNrTestOK} ]
 do
  echo " ${totalNameTestOK[$i]}"
  ((i++))
 done

 if [ ${totalNrTestFAIL} -gt 0 ]; then
  echo
  echo "Tests that FAILED:"
  i=0
  while [ $i -lt ${totalNrTestFAIL} ]
  do
   echo " ${totalNameTestFAIL[$i]}"
   ((i++))
  done
 fi

 echo
 if [ ${totalNrTestFAIL} -gt 0 ]; then
  echo "Overall result is FAILED"
  exit ${REGTEST_FAIL}
 else
  echo "Overall result is OK"
  exit ${REGTEST_OK}
 fi 
}

while [ $# -gt 0 ]
do
 case $1 in
 --sehost | -sehost ) sehost=$2
  shift 
  ;;
 --lfchost | -lfchost ) lfchost=$2
  shift
  ;;
 --lfcdir | -lfcdir ) lfcdir=$2
  shift
  ;;
 --vo | -vo ) voname=$2
  shift
  ;;
 --help | -help | --h | -h ) usage
  exit 0
  ;;
 --* | -* ) echo "$0: invalid option $1" >&2
  usage
  exit 1
  ;;
 *) break
  ;;
 esac
 shift
done

totalNrTest=0
totalNrTestOK=0
totalNrTestFAIL=0

for testCategory in $*
do
 if [ $testCategory = "UI-data-lcg" ]; then
  options="-d $sehost"
  export LFC_HOST=$lfchost
  if [ $voname ]; then
   options="$options --vo $voname"
  fi
 elif [ $testCategory = "UI-data-lfc" ]; then
   if [ $lfchost ]; then
    options="--lfchost $lfchost"
   fi
   if [ $lfcdir ]; then
    options="$options --lfcdir $lfcdir"
   fi
 elif [ $testCategory = "UI-inf-lcg-info" ]; then
  if [ $voname ]; then
   options="--vo $voname"
  fi
 fi
 for testName in ${testCategory}*
 do
  ./$testName $options
  testResult=$?
  record_result
  ((totalNrTest++))
 done
done

total_test_result
