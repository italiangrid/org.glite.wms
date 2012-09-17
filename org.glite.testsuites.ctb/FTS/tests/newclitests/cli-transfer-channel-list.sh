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
# AUTHORS: Gianni Pucciani CERN 
##############################################################################

#source functions definitions
source FTS-common
shortinfo="Test CLI glite-transfer-channel-list"

showUsage()
{
 echo <<EOF
 Usage: $0 [--fts <fts hostname>] [--shortinfo] [--longinfo]
 Options: 
   <fts hostname> The name of the FTS host
   --shortinfo = print a short test description
   --longinfo = print a long test description
EOF

}

function test_transfer-channel-list-parameters()
{

    echo "Check parameters: -q -v -x"

    echo "Check with incorrect service"
    run_command_status glite-transfer-channel-list -s https://FTS_HOST:8443/glite-data-transfer-fts/services/ChannelManagement 1 || return 2

    echo "Check -v  parameter" 
    run_command_status glite-transfer-channel-list -v -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/ChannelManagement $CHANNEL 0 || return 2

    echo "Check -q, quite operation" 
    run_command_status glite-transfer-channel-list -q  -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/ChannelManagement $CHANNEL 0 || return 2

    echo "Check -x  parameter" 
    run_command_status glite-transfer-channel-list -x  -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/ChannelManagement $CHANNEL 0 || return 2
    echo $OUTPUT | grep "Srm copy" > /dev/null
    if [ $? != 0 ]; then
      echo "Error: extended information not printed"
      return 1
    fi

    echo "Check non existing channel" 
    run_command_status glite-transfer-channel-list -x  -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/ChannelManagement DUMMYCHANNEL 1 || return 2

    return 0
}

function pretest() {
  
run_function check_valid_proxy || return 1

run_function simple-channel-add TEST-TEST TEST1 TEST2 || return 1

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
run_test test_transfer-channel-list-parameters $PASSED || failed=1

run_function posttest || exit 1

if [ $failed -eq 1 ]; then
  echo ""
  echo "$shortinfo: Failed"
  exit 1
else
  echo ""
  echo "$shortinfo: Passed"
  exit 0
fi


