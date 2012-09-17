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
shortinfo="Test CLI glite-transfer-channel-audit"

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

# FTS_HOST must be set, channel name as first argument.
# Added by Victor Galaktionov 24/11/209
function test_transfer-channel-audit-parameters()
{
  echo "Check parameters: -q -v "

  echo "Test -q parameter"
  run_command_status glite-transfer-channel-audit -q 1 || return 1
  if [ "x$OUTPUT" != "x" ]; then
    echo "Error: the command returned this output '$OUTPUT'"
    return 2
  fi

  echo "Test -v  parameter" 
  run_command_status glite-transfer-channel-audit -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/ChannelManagement $Channel -v 0 || return 1
  echo $OUTPUT | grep -c "Service version: "
  if [ $? -ne 0 ]; then
    echo "Error: 'Service version' not present in the output"
    return 2
  fi
  

  return 0
}

# FTS_HOST must be set, channel name as first argument.
function test_transfer-channel-audit()
{
  echo "Check basic functionality"
  echo "Test basic command"
  run_command_status glite-transfer-channel-audit -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/ChannelManagement $CHANNEL 0 || return 1
  echo $OUTPUT | grep -q "Channel: $CHANNEL"
  if [ $? -ne 0 ]; then
    echo "Error: Channel info missing in command output"
    return 2
  fi

  echo "Test -r state"
  run_command_status glite-transfer-channel-audit  -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/ChannelManagement -r state $CHANNEL 0 || return 1
  echo $OUTPUT | grep -q "State: "
  if [ $? -ne 0 ]; then
    echo "Error: State info missing in command output"
    return 2
  fi

  return 0
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
      echo "  - Test basic audit functionality on the existing channel CERN-CERN"
      echo "  - Test arguments"
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

run_function channel-exists $CHANNEL result || return 1
if [ "x$result" != "xyes" ]; then
  echo "channel-exists did not return the expected result: $result"
  return 1
fi
echo " pretest OK"
return 0
}

CHANNEL="CERN-CERN"
run_function pretest || exit 1
    
failed=0
run_test test_transfer-channel-audit $PASSED || failed=1

run_test test_transfer-channel-audit-parameters $PASSED || failed=1

if [ $failed -eq 1 ]; then
  echo ""
  echo "$shortinfo: Failed"
  exit 1
else
  echo ""
  echo "$shortinfo: Passed"
  exit 0
fi

run_function posttest || return 1

echo "-TEST PASSED-"
exit 0
