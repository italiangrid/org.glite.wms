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

shortinfo="Test CLI glite-transfer-channel-add"

#source functions definitions
source FTS-common

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



# FTS_HOST must be set.
# Added by Victor Galaktionov 24/11/209
function test_transfer-channel-add-parameters
{
  echo "Check parameters: -q -v "

  echo "Check -q, quite operation"
  run_command_status glite-transfer-channel-add  -q -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/ChannelManagement TEST-TEST TEST1 TEST2 0 || return 2
  run_function channel-exists TEST-TEST result || return 1
  if [ "x$result" == "xno" ]; then
    echo "Error: channel not created"
    return 2
  elif [ "x$result" != "xyes" ]; then
    echo "Error: could not verify the creation of the channel, answer was: $result"
    return 1
  else
    echo "Channel correctly created"
  fi
  run_function channel-drop TEST-TEST || return 1

  echo "Check -v, Print details about the service"
  run_command_status glite-transfer-channel-add -v -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/ChannelManagement 1 || return 2
  echo $OUTPUT | grep -q "Service version:"
  if [ $? -ne 0 ]; then
    echo "Service version not printed"
    return 2
  fi
  return 0
}

function test_transfer-channel-add() {
    
  local BANDWIDTH=10
  local CONTACT="my.contact@cern.ch"
  local STATE=Stopped
  local STREAMS=2
  local FILES=2
  local THROUGHPUT=10
  local SITE_S=SITEA
  local SITE_D=SITEB
  localNOTAUTH="You are not authorised"
    
  echo "1. Add channel"
  run_command_status glite-transfer-channel-add  -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/ChannelManagement --verbose \
    -b $BANDWIDTH -c $CONTACT  -S $STATE -T $STREAMS -f $FILES -t $THROUGHPUT $CHANNEL $SITE_S $SITE_D 0 || return 1

  echo "2. Check added channel"
  run_command_status glite-transfer-channel-list  -x -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/ChannelManagement $CHANNEL 0 || return 1
  
  declare -a OPTIONS
  OPTIONS=( $CHANNEL $BANDWIDTH  $CONTACT $STATE $STREAMS $FILES $THROUGHPUT)

  for item in ${OPTIONS[*]}
  do
    echo $OUTPUT || grep -q $item 
    if [ $? != 0 ]; then
      echo "Channel property incorrect"
      return 1
    fi
  done

  echo "3. Checkng failure adding existing channel"
  run_command_status glite-transfer-channel-add  -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/ChannelManagement $CHANNEL  $SITE_S $SITE_D 1 || return 1

  echo "4. drop added channel"
  run_function channel-drop TEST-TEST || return 1
  echo "Channel correctly dropped"

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
      echo "  - "
      echo "  - "
      echo "  - "
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
if [ "x$result" != "xno" ]; then
  echo "channel-exists did not return the expected result: $result"
  return 1
fi
echo " pretest OK"
return 0
}

function posttest() {

glite-transfer-channel-drop  -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/ChannelManagement $CHANNEL || return 1
echo " posttest ok"
return 0
}

CHANNEL="TEST-TEST"
run_function pretest || exit 1

failed=0
run_test test_transfer-channel-add $PASSED || failed=1

run_test test_transfer-channel-add-parameters $PASSED || failed=1

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

