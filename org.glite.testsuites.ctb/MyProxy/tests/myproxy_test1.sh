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
Usage: myproxy-test1.sh [--options] MyProxy_HOST
   1. Store a credential (myproxy-init).
   2. Get info on the stored credential (myproxy-info)
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
let passphrase=passphrase+1000001
echo $passphrase | myproxy-init -v -a -c 1 -t 1 -S 2>&1
if [ "$?" -ne 0 ]
then
   myexit 1 "myproxy-init"
fi  
  myecho "myproxy-init is succeeded"
run_command myproxy-info -v
  myecho "myproxy-info is succeeded"
run_command myproxy-destroy -v -s $MYPROXY_SERVER
   myecho "myproxy-destroy is succeeded"
rm -f /tmp/myproxy-test.$$.*
myexit 0
