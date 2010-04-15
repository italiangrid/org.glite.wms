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
Usage: myproxy-test3.sh [--options] MyProxy_HOST
   1. Store credentials with retrieval policies. 
   2. Verify retrieval policies (one accept, one deny).
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
ERR=0
passphrase=`date +%s`
passphrase=$RANDOM
let passphrase=passphrase+1000001
echo $passphrase | myproxy-init -v -r "nobody" -k "nobody" -c 1 -t 1 -S 2>&1
if [ "$?" -ne 0 ]
then
   ERR=1
   myecho  "failed: myproxy-init -v -r "nobody" -k "nobody" -c 1 -t 1 -S "
else  
  myecho "succeeded: myproxy-init -v -r "nobody" -k "nobody" -c 1 -t 1 -S "
fi
echo $passphrase | myproxy-init -v -x -r "$cert_subject" -k "mine" -c 1 -t 1 -S 2>&1
if [ "$?" -ne 0 ]
then
   ERR=1
   myecho  "failed: myproxy-init  -v -x -r "$cert_subject" -k "mine" -c 1 -t 1 -S "
else  
  myecho "succeeded: myproxy-init -v -x -r "$cert_subject" -k "mine" -c 1 -t 1 -S "
fi
 echo $passphrase | myproxy-logon -k "mine" -t 1 -o /tmp/myproxy-test.$$ -v -S   2>&1
 if [ "$?" -ne 0 ]
then
   ERR=1
   myecho "failed: myproxy-logon -k "mine" -t 1 -o /tmp/myproxy-test.$$ -v -S "
else  
  myecho "succeeded: myproxy-logon -k "mine" -t 1 -o /tmp/myproxy-test.$$ -v -S"
  run_command verifyproxy /tmp/myproxy-test.$$
fi
 echo $passphrase | myproxy-logon -k "nobody" -t 1 -o /tmp/myproxy-test.$$ -v -S 2>&1
 if [ "$?" =  0 ]
then
   ERR=1
   myecho "failed: myproxy-logon -k "nobody" -t 1 -o /tmp/myproxy-test.$$"
else
  myecho "succeeded: myproxy-logon -k "nobody" -t 1 -o /tmp/myproxy-test.$$"
fi  
   myproxy-destroy -v -k "mine" -s $MYPROXY_SERVER
    if [ "$?" -ne 0 ]
then
   ERR=1
   myecho "failed: myproxy-destroy -v -k "mine" -s $MYPROXY_SERVER"
else
   myecho "succeeded: myproxy-destroy -v -k "mine" -s $MYPROXY_SERVER"
fi
   myproxy-destroy -v -k "nobody" -s $MYPROXY_SERVER
  if [ "$?" -ne 0 ]
then
   ERR=1
   myecho  "failed: myproxy-destroy -v -k "nobody" -s $MYPROXY_SERVER"
else
   myecho "succeeded: myproxy-destroy -v -k "nobody" -s $MYPROXY_SERVER"
fi
if [ "$ERR" = "1" ]; then
  rm -f /tmp/myproxy-test.$$ 2>&1
  rm -f /tmp/tmpproxy.$$ 2>&1
  myecho "end time: " `date`
  echo "*** -TEST FAILED- ***"
  exit 2
fi
  rm -f /tmp/myproxy-test.$$ 2>&1
  rm -f /tmp/tmpproxy.$$ 2>&1
  myecho "end time: " `date`
  echo "=== test PASSED ==="
exit 0
