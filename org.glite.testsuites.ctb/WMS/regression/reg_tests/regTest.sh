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
. config/config.sh
. config/commonFunctions.sh

error() {
 echo "$@" 1>&2
 usage_and_exit 1
}

usage() {
 echo "Usage: $PROGRAM --testlist <file with list of bugs> | --pretestlist <file with list of bugs>"
}

usage_and_exit() {
 usage
 exit $1
}

PROGRAM=`basename $0`

pretest=false

while test $# -gt 0
 do
  case $1 in
   --testlist | -testlist | --tl | -t )
    shift
    testlist="$1" 
   ;;
   --help | -help | -h )
    usage_and_exit 0
   ;;
   --pretestlist | -pretestlist | --ptl | -ptl )
    pretest=true
    shift
    testlist="$1"
   ;;
   -*)
    error "Unrecognised option: $1"
   ;;
   *)
    break
   ;;
  esac
 shift 
done

#### Function total_test_result

function total_test_result() {
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
  echo -n " ${totalNameTestOK[$i]}"
  ((i++))
 done

 if [ ${totalNrTestFAIL} -gt 0 ]; then
  echo
  echo "Tests that FAILED:"
  i=0
  while [ $i -lt ${totalNrTestFAIL} ]
  do
   echo -n " ${totalNameTestFAIL[$i]}"
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

#### End of function total_test_result

#### Function record_result

record_result() {
 if [ $testResult -eq ${REGTEST_OK} ]; then
  echo "Test for bug ${bugNr} OK"
  totalNameTestOK[${totalNrTestOK}]=${bugNr}
  ((totalNrTestOK++))
 elif [ $testResult -eq $REGTEST_TIMEOUT_ERROR ]; then
  echo "Test for bug ${bugNr} FAILED because of TIMEOUT"
  totalNameTestFAIL[${totalNrTestFAIL}]=${bugNr}
  ((totalNrTestFAIL++))
 else
  echo "Test for bug ${bugNr} FAILED"
  totalNameTestFAIL[${totalNrTestFAIL}]=${bugNr}
  ((totalNrTestFAIL++))
 fi
}

#### End of function record_result

if [ ! -e "$testlist" ]; then
 error "File ${testlist} does not exist."
fi

if [ ! -e ${testlist%.*}.sh ]; then
 error "File ${testlist%.*}.sh does not exist."
fi

echo "Using file ${testlist} for the list of bugs to test."

. ${testlist%.*}.sh


totalNrTest=0
totalNrTestOK=0
totalNrTestFAIL=0
testResult=0

if [ $pretest = "true" ]; then
 while read bugNr arg
 do
  if [ -z $(echo $bugNr | grep '#') ]; then
   if [ -f $REGTEST_BUGTESTDIR/bug${bugNr} ]; then
    echo
    echo "============================================="
    echo "Executing pretest for bug ${bugNr}"
    if [ -f config/bug${bugNr}_config.sh ]; then
     . config/bug${bugNr}_config.sh
    fi
    . $REGTEST_BUGTESTDIR/bug${bugNr}
    test_bug${bugNr}_pre
    testResult=$?
    ((totalNrTest++))
    record_result
   else
    echo "current dir: `pwd`" 
    echo "The test for bug $bugNr does not exist. Cannot find file $REGTEST_BUGTESTDIR/bug${bugNr}"
    echo "Terminating regression pretest run."
    exit ${REGTEST_FAIL}
   fi
  fi
 done < $testlist
 total_test_result
fi


while read bugNr arg
do
 if [ -z $(echo $bugNr | grep '#') ]; then
  if [ -f $REGTEST_BUGTESTDIR/bug${bugNr} ]; then
   echo
   echo "============================================="
   echo "Starting test for bug ${bugNr}"
   if [ -f config/bug${bugNr}_config.sh ]; then
    . config/bug${bugNr}_config.sh
   fi
   . $REGTEST_BUGTESTDIR/bug${bugNr}
   test_bug${bugNr}_pre
   if [ $? -eq ${REGTEST_FAIL} ]; then
    echo "Execution of test_bug${bugNr}_pre failed."
    echo "Terminating regression test run."
    exit $REGTEST_FAIL
   fi
   test_bug${bugNr} ${arg} 
   testResult=$?
   test_bug${bugNr}_post
   ((totalNrTest++))
  else
   echo "current dir: `pwd`"
   echo "The test for bug $bugNr does not exist. Cannot find file $REGTEST_BUGTESTDIR/bug${bugNr}"
   echo "Terminating regression test run."
   exit $REGTEST_FAIL
  fi
  record_result 
 fi
done < $testlist

total_test_result

if [ ${totalNrTestFAIL} -ge 0 ]; then
 exit $REGTEST_EXIT_FAIL
fi

exit $REGTEST_EXIT_OK
