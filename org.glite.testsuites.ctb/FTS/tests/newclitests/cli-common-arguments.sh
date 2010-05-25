#!/bin/bash

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
# AUTHORS:
# Gianni Pucciani
#
##############################################################################

#source functions definitions
source FTS-common

shortinfo="Test CLI common arguments"

#Script for testing FTS channel add operation
showUsage()
{
 echo << EOF
 Usage: $0 [--fts <fts hostname>] [--shortinfo] [--longinfo]
 Options:  
   <fts hostname> The name of the FTS host
   --shortinfo = print a short test description
   --longinfo = print a long test description
EOF
}

until [ -z "$1" ] 
do
  case "$1" in
     --fts)
      if [ -z "$2" ]; then
        shift 1
      else
        FTS_HOST=$2
        shift 2
      fi
     ;;
     --shortinfo )
      echo $shortinfo
      exit 0
    ;;
    --longinfo ) 
      echo "$shortinfo: "
      echo "  - For each command available, test the common arguments:"
      echo "  - -h, --help Print this help text and exit"
      echo "  - -V Print the version number and exit"
      exit 0
    ;;
    *)
     showUsage
     exit 2
    ;; 
  esac
done 

function pretest() {

  run_function check_valid_proxy || return 1
  return 0
}

function test_cli-common-arguments()
{
  echo "Check parameters: -h, --help"

  for command in $COMMANDS;
  do
    echo "Test -h parameter"
    run_command_status $command -h 0 || return 1
    if [ "x$OUTPUT" == "x" ]; then
      echo "Error: the command returned no output"
      return 1
    fi

    echo "Test --help parameter"
    run_command_status $command --help 0 || return 1
    if [ "x$OUTPUT" == "x" ]; then
      echo "Error: the command returned no output"
      return 1
    fi

    echo "Test -V parameter" 
    run_command_status $command -V 0 || return 1
    if [ "x$OUTPUT" == "x" ]; then
      echo "Error: the command returned no output"
      return 1
    fi
  done
  return 0
}

run_function pretest || exit 1

failed=0
run_test test_cli-common-arguments $PASSED || failed=1

if [ $failed -eq 1 ]; then
  echo ""
  echo "$shortinfo: Failed"
  exit 1
else
  echo ""
  echo "$shortinfo: Passed"
  exit 0
fi

echo "-TEST PASSED-"
exit 0

