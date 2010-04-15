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

# Start of myproxy-store and myproxy-retrieve tests
#

# commands to test: myproxy-store, myproxy-info, myproxy-destroy,
#                   myproxy-logon, myproxy-retrieve, and
#                   myproxy-change-pass-phrase

# For myproxy-store, we need an encrypted key to store.
showUsage ()
{
cat <<EOF
Usage: myproxy-test6.sh [--options] MyProxy_HOST
   1. Store credential (myproxy-store -v -t 1) 
   2. Get info on the stored credential (myproxy-info)
   3. Create proxy from stored credential (myproxy-logon).
   4. Retrieve stored credential (myproxy-retrieve)
   5. Verify passphrase checking (myproxy-logon).
   6. Verify passphrase checking (myproxy-retrieve).
   7. Verify renewal fails by default (myproxy-logon).
   8. Verify renewal fails by default (myproxy-retrieve).
   9. Remove credential from repository (myproxy-destroy). 
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

. setup_env
. myproxy-functions.sh
MYPROXY_SERVER=$1
if [ "$MYPROXY_SERVER" == "" ]
then 
  myexit 1 "MYPROXY_SERVER is not defined"
fi 
vomsproxyinfo
get_cert_subject
echo cert_subject=$cert_subject
passphrase=`date +%s`
passphrase=$RANDOM
trustrootdir="$HOME/.globus/certificates.test.$$"
let passphrase=passphrase+1000100111
# So, let's encrypt our proxy key.
testkey="/tmp/myproxy-test.$$.key"
echo X509_USER_KEY =  $X509_USER_KEY
echo $passphrase | openssl rsa -des3 -passout stdin -in $X509_USER_KEY -out $testkey 2>&1
chmod 0600 $testkey
run_command myproxy-store -x -E \"$cert_subject\" -v -t 1 -y $testkey
if [ "$?" -ne 0 ]
then
   myecho "Server does not support myproxy-store."
   myecho "failed: myproxy-store -x -E \"$cert_subject\" -v -t 1 -y $testkey ."
else
   
   myecho "succeeded: myproxy-store -x -E \"$cert_subject\" -v -t 1 -y $testkey"
#   echo "***** get info for stored credential *****"
run_command   myproxy-info -v
   if [ "$?" -eq 0 ]
 then
   myecho "succeeded: info for stored credential is succeeded"
 else
   myecho "failed: info for stored credential is failed"
   myexit 1 "myproxy-info -v"
  fi
fi 
echo $passphrase | myproxy-logon -t 1 -o /tmp/myproxy-test.$$ -v -S 2>&1  
 if [ "$?" = "0" ]
then
  myecho "succeeded: myproxy-logon myproxy-logon -t 1 -o /tmp/myproxy-test.$$ -v " 
  verifyproxy /tmp/myproxy-test.$$
if [ "$?" -ne "0" ]; then
  myexit 1 "verifyproxy /tmp/myproxy-test.$$"
fi
else 
  rm -f /tmp/myproxy-test.$$ 2>&1
  rm -f /tmp/tmpproxy.$$ 2>&1
  myexit 1 "myproxy-logon -t 1 -o /tmp/myproxy-test.$$ -v -S"
fi
echo $passphrase | myproxy-retrieve -c /tmp/myproxy-test.cert.$$ -y /tmp/myproxy-test.key.$$  -S 2>&1
if [ "$?" = "0" ]  ; then
verifycert "/tmp/myproxy-test.cert.$$" "/tmp/myproxy-test.key.$$"
if [ $? != "0" ];then
  rm -f /tmp/myproxy-test.$$ 2>&1
  rm -f /tmp/tmpproxy.$$ 2>&1
myexit 1, "verifycert /tmp/myproxy-test.cert.$$.pem /tmp/myproxy-test.key.$$.pem"
else
 myecho "succeeded: verifycert /tmp/myproxy-test.cert.$$.pem /tmp/myproxy-test.key.$$.pem"
fi
else
  rm -f /tmp/myproxy-test.$$ 2>&1
  rm -f /tmp/tmpproxy.$$ 2>&1
myexit 1 "myproxy-retrieve -c /tmp/myproxy-test.cert.$$ -y /tmp/myproxy-test.key.$$"
fi
X509_CERT_DIR=$trustrootdir
echo $passphrase | myproxy-retrieve -T -c /tmp/myproxy-test.cert.$$ -y /tmp/myproxy-test.key.$$  -S 2>&1
if [ $? != "0" ];then
verifycert "/tmp/myproxy-test.cert.$$" "/tmp/myproxy-test.key.$$"
if [ $? != "0" ];then
  rm -f /tmp/myproxy-test.$$ 2>&1
  rm -f /tmp/tmpproxy.$$ 2>&1
myexit 1, "verifycert /tmp/myproxy-test.cert.$$.pem /tmp/myproxy-test.key.$$.pem"
else
 myecho "succeeded: verifycert /tmp/myproxy-test.cert.$$.pem /tmp/myproxy-test.key.$$.pem"
fi
else
  rm -f /tmp/myproxy-test.$$ 2>&1
  rm -f /tmp/tmpproxy.$$ 2>&1
myexit 1 "myproxy-retrieve -T -c /tmp/myproxy-test.cert.$$ -y /tmp/myproxy-test.key.$$"
fi
run_command myproxy-destroy -v -s $MYPROXY_SERVER
  rm -f /tmp/myproxy-test.$$ 2>&1
  rm -f /tmp/tmpproxy.$$ 2>&1
myexit 0
