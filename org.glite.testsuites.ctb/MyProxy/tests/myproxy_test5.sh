#!/bin/bash
##############################################################################
# Copyright (c) Members of the EGEE Collaboration. 2010.
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
# AUTHOR Liudmila Stepanova, SINP MSU
#
##############################################################################
showUsage ()
{
cat <<EOF
Usage: myproxy-test5.sh [--options] MyProxy_HOST
   1. Verify lifetime of retrieved credentials.
   2. Remove credential from repository (myproxy-destroy).
  Options :
    -help                           displays usage
    -h                              displays usage
    --help                          displays usage
EOF
}
if [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$1" = "--help" ]; then
  showUsage
  exit 1

fi

MYPROXY_SERVER=$1
. setup_env
. myproxy-functions.sh
if [ "$MYPROXY_SERVER" == "" ]
then 
  myexit 1 "MYPROXY_SERVER is not defined"
fi 
vomsproxyinfo
get_cert_subject
passphrase=`date +%s`
passphrase=$RANDOM
let passphrase=passphrase+1000001
echo $passphrase | myproxy-init -v -a -c 3 -t 2 -S 2>&1
if [ "$?" -ne 0 ]
then
   myexit 1 " myproxy-init -v -a -c 3 -t 2 "
fi  
  myecho "succeeded: myproxy-init -v -a -c 3 -t 2 "

echo $passphrase |  myproxy-logon -o /tmp/myproxy-test.$$ -v -S   2>&1
 if [ "$?" -ne 0 ]
then
   myexit 1 "myproxy-logon -o /tmp/myproxy-test.$$ -v"
fi  
  myecho "succeeded: myproxy-logon -o /tmp/myproxy-test.$$ -v"
timeleft=`grid-proxy-info -file /tmp/myproxy-test.$$ -timeleft  2>&1`

let timeleftmax=60*60*2+300
let timeleftmin=60*60*2-300
echo $timeleftmin  $timeleft  $timeleftmax
if [ "$timeleft" -gt "$timeleftmax" ] || [ "$timeleft" -lt "$timeleftmin" ]; then
ERR=1
myecho "failed: Credential lifetime $timeleft incorrect."
else
 run_command verifyproxy /tmp/myproxy-test.$$
echo $passphrase |  myproxy-logon -t 1 -o /tmp/myproxy-test.$$ -v -S   2>&1
timeleft=`grid-proxy-info -file /tmp/myproxy-test.$$ -timeleft  2>&1`
 let timeleftmax=60*61
 let timeleftmin=60*59
 if [ "$timeleft" -gt "$timeleftmax" ] || [ "$timeleft" -lt "$timeleftmin" ]; then
 myexit 1 "Credential lifetime $timeleft incorrect."
 fi
fi 
#echo $timeleftmin  $timeleft  $timeleftmax
run_command verifyproxy /tmp/myproxy-test.$$

run_command myproxy-destroy -v  -s $MYPROXY_SERVER
   myecho "succeeded: myproxy-destroy"
if [ "$ERR" = "1" ]; then
   myecho "failed: Credential lifetime $timeleft incorrect."
  rm -f /tmp/myproxy-test.$$ 2>&1
  rm -f /tmp/tmpproxy.$$ 2>&1
   myexit 1
fi
  rm -f /tmp/myproxy-test.$$ 2>&1
  rm -f /tmp/tmpproxy.$$ 2>&1    
myexit 0
