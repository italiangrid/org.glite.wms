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
 echo "Test driver for executing several UI tests in one go. You can run all the tests or a subset of them. Running test-lcg-utils.sh is not supported."
 echo "Usage:"
 echo "======"
 echo "UI-test-driver.sh --platform <gl32-SLC5-x86_64|gl31-SLC4-i386> --sehost|-sehost <SE hostname> --lfchost|-lfchost <LFC hostname> --lfcdir|-lfcdir <LFC directory> --vo|-vo <VO name> --cetag <CE hostname for tag check> [--sctag <subcluster for tag checks>] [--extended] [--all] [TestCategories]"
 echo "Test categories can be one or more of the following"
 echo "UI-environment UI-glite-environment UI-commands UI-libraries"
 echo "UI-manpage UI-exec UI-security UI-data-lcg UI-data-lfc" 
 echo "UI-inf-lcg-info UI-workload-glite-wms UI-tags"
 echo ""
 echo "Some test categories need to/can be provided with options:"
 echo "UI-security: --vo (mandatory)"
 echo "UI-inf-lcg-info options: --vo (optional)"
 echo "UI-data-lcg options: --sehost (mandatory), --vo (optional)"
 echo "UI-data-lfc options: --lfchost (mandatory if LFC_HOST is not defined in your environment), --lfcdir (optional)"
 echo "UI-inf-lcg-info options: --vo (optional)"
 echo "UI-tags: --vo (mandatory), --cetag (mandatory), --sctag (optional, use this to test the the --sc flag too)"
 echo "          --extended (optional, use if you have permission to modify tags on target ce) "
 echo ""
 echo "The --platform argument is mandatory"
 echo "The --all option will run all tests. This requires all test specific mandatory arguments"
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
 --platform | -platform ) platform=$2
  shift
  ;;
 --all | -all ) test_list="UI-environment UI-glite-environment UI-commands UI-libraries UI-manpage UI-exec UI-security UI-data-lcg UI-data-lfc UI-inf-lcg-info UI-workload-glite-wms"
  all_tests=y
  ;;
 --vo | -vo ) voname=$2
  shift
  ;;
 --cetag | -cetag ) cetag=$2
  shift
  ;;
 --sctag | -sctag ) sctag=$2
  shift
  ;;
 --extended | -extended ) extended="--extended"
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

if [ "x$platform" == "x" ] ; then
 echo "Mandatory argument --platform missing"
 exit 1
else
 if [ $platform != "gl31-SLC4-i386" ] && [ $platform != "gl32-SLC5-x86_64" ] ; then
  echo "Invalid platform $platform specified"
  exit 1
 fi
fi

if [ x$all_tests != xy ] ; then
  test_list=$*
fi

totalNrTest=0
totalNrTestOK=0
totalNrTestFAIL=0

cd `pwd`/${platform}

for testCategory in $test_list
do
 options=""
 # the environment test are special, they need to be run with a shell
 if [ ${testCategory} = "UI-environment" ] ; then
  testName=UI-environment.csh
  tcsh ../common/$testName
  testResult=$?
  record_result
  ((totalNrTest++))

  testName=UI-environment.sh
  sh ../common/$testName
  testResult=$?
  record_result
  ((totalNrTest++))
  continue
 fi
 if [ $testCategory = "UI-glite-environment" ] ; then
  testName=UI-glite-environment.csh
  tcsh ../common/$testName
  testResult=$?
  record_result
  ((totalNrTest++))

  testName=UI-glite-environment.sh
  sh ../common/$testName
  testResult=$?
  record_result
  ((totalNrTest++))
  continue
 fi
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
 elif [ $testCategory = "UI-security" ]; then
  if [ $voname ]; then
   options="-voms $voname"
  fi
 elif [ $testCategory = "UI-tags" ]; then
   options="--vo $voname --ce $cetag $extended"
   if [ $sctag ] ; then
    options="$options --sc $sctag" 
   fi 
 fi
 for testName in ${testCategory}*
 do
  if [ -e $testName ] ; then
   ./$testName $options
   testResult=$?
   record_result
   ((totalNrTest++))
  fi
 done
 for testName in ../common/${testCategory}*
 do
  if [ -e $testname ] ; then
   ./$testName $options
   testResult=$?
   record_result
   ((totalNrTest++))
  fi
 done
done

total_test_result
