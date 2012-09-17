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
Usage: myproxy-test4.sh [--options] MyProxy_HOST
   1. Store credentials with renewal policies.
   2. Verify renewal policies (one accept, one deny).
   3. Remove credential from repository (myproxy-destroy).
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
let passphrase=passphrase+1010010011
echo $passphrase | myproxy-init -v  -R "nobody" -k "nobody" -c 1 -t 1 -d -S 2>&1
if [ "$?" -ne 0 ]
then
   myexit 1 "myproxy-init -v  -R "nobody" -k "nobody" -c 1 -t 1 -d"
fi  
  myecho "succeeded myproxy-init -v  -R "nobody" -k "nobody" -c 1 -t 1 -d"
echo cert_subject=$cert_subject  myproxy-init -v -x -R \"$cert_subject\" -k "mine" -c 1 -t 1 -d
echo $passphrase | myproxy-init -v -x -R "$cert_subject" -k "mine" -c 1 -t 1 -d -S 2>&1
if [ "$?" -ne 0 ]
then
  rm -f /tmp/myproxy-test.$$ 2>&1
  rm -f /tmp/tmpproxy.$$ 2>&1
   myexit 1 "myproxy-init -v -x -R "$cert_subject" -k "mine" -c 1 -t 1 -d "
fi  
  myecho "succeeded: myproxy-init -v -x -R "$cert_subject" -k "mine" -c 1 -t 1 -d "
  run_command myproxy-info -v -d
  myecho "succeeded: myproxy-info"
  myproxy-logon -k "mine" -a $X509_USER_PROXY -t 1 -o /tmp/myproxy-test.$$ -v -d   2>&1
 if [ "$?" -ne 0 ]
then
   myecho "Shouldn't have allowed retrieval"
  rm -f /tmp/myproxy-test.$$ 2>&1
  rm -f /tmp/tmpproxy.$$ 2>&1

   myexit 1 "myproxy-logon -k "mine" -a $X509_USER_PROXY -t 1 -o /tmp/myproxy-test.$$ -v -d"
fi  
  myecho "myproxy-logon -k "mine" -a $X509_USER_PROXY -t 1 -o /tmp/myproxy-test.$$ -v -d is succeeded"
run_command verifyproxy /tmp/myproxy-test.$$
myecho "succeeded: verify proxy"
myproxy-logon -k "nobody" -a $X509_USER_PROXY -t 1 -o /tmp/myproxy-test.$$ -v -d   2>&1
 if [ "$?" = "0" ]
then
  rm -f /tmp/myproxy-test.$$ 2>&1
  rm -f /tmp/tmpproxy.$$ 2>&1
   myexit 1 "myproxy-logon -a $X509_USER_PROXY -t 1 -o /tmp/myproxy-test.$$ -v -d"
fi  
  myecho "succeeded: myproxy-logon -a $X509_USER_PROXY -t 1 -o /tmp/myproxy-test.$$ -v -d"
run_command myproxy-destroy -v -k "mine" -d -s $MYPROXY_SERVER
   myecho "myproxy-destroy is succeeded"
run_command myproxy-destroy -v -k "nobody" -d -s $MYPROXY_SERVER
   myecho "myproxy-destroy is succeeded" 
  rm -f /tmp/myproxy-test.$$ 2>&1
  rm -f /tmp/tmpproxy.$$ 2>&1
  
myexit 0
