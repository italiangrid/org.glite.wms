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

function myecho() {
  echo " ===> $@"
}

function myexit() {
  echo ""
  myecho "end time: " `date`
  echo ""

  if [ $? -ne 0 ] || [ "$1" != "0" ]; then

    echo " *** -TEST FAILED- *** "
    if [ "$1" != "0" ]; then
      echo " *** failed command: $2 *** "
    fi
    exit 2
  fi

  echo "    === test PASSED === "
  exit 0
}

function run_command() {
  echo ""
  echo " $" $@

  OUTPUT=$(eval $@ 2>&1)
  if [ $? -gt 0 ]; then
   echo "${OUTPUT}"
    myecho "$1 failed"
    echo ""
    myexit 2 $1
  fi

  echo "${OUTPUT}"

  return 0
}
function vomsproxyinfo() {
timeleft=`voms-proxy-info -timeleft 2>&1`
if [ "$?" != 0 ];then
myecho "Couldn't find a valid proxy. "
myexit 1 "voms-proxy-info -timeleft"
fi
if [ "$timeleft" -lt 60 ]
then
   echo
   myecho "failed: proxy is expired "
   echo " *** -TEST FAILED- *** "
   exit 2
fi
return 0
} 

function verifyproxy() {
timeleft=`grid-proxy-info -file $1 -timeleft 2>&1`
#echo TIMELEFT=$timeleft
if [ "$timeleft" == "" ]
then
  echo
  echo "***** failed to verify proxy; 'grid-proxy-info -timeleft' failed *****"
  echo
fi
if [ "$timeleft" -lt 1 ]
then
   echo
   echo "***** proxy is expired *****"
   echo
   exit 1
fi
  proxytype=`grid-proxy-info -file $1 -type`
#echo  PROXYTYPE=$proxytype
  proxytype=`echo $proxytype | awk '{ print $2 }'`
  if [ "$proxytype" = "legacy" ]
then
   typeopt="-old"
else
   $="-rfc"
fi
output=`grid-proxy-init $typeopt -debug -verify -cert $1 -key $1 -valid 0:1 -out /tmp/tmpproxy.$$ 2>&1`
if [ "$?" -ne 0 ]
then
   echo
   echo "****** failed to verify proxy ******"
   echo $output
   echo
if [ -e "/tmp/tmpproxy.$$" ]
then
   rm -f "/tmp/tmpproxy.$$"
fi
   exit 1
fi
return 0
}

function get_cert_subject(){
cert_subject=`grid-proxy-info -subject 2>&1`
#cert_subject=${cert_subject%/CN=proxy}
cert_subject=`awk -F"/CN=proxy" '{ print $1 }'<<< $cert_subject`
#cert_subject=${cert_subject%/CN=limited proxy}
cert_subject=`awk -F"/CN=limited proxy" '{ print $1 }'<<< $cert_subject`
cert_subject=${cert_subject%/CN=[0-9]*}
return 0
}
function verifycert() {
certfile=$1
keyfile=$2
info=`grid-cert-info -file $certfile 2>&1`
if [ "$?" -ne "0" ]; then
echo $cert_subject
exit 1
fi
old=`awk  '/CN=proxy/||/CN=limited proxy/{ print $RLENGTH }' <<< $info`
if [ "$old" = "" ];then
    proxytype="-rfc"
else
    proxytype="-old"
fi
echo $passphrase | grid-proxy-init $proxytype -debug -verify -cert $certfile -key $keyfile -valid 0:1 -out /tmp/tmpproxy.$$ -pwstdin
if [ "$?" -ne "0" ]; then
voms-proxy-info -file /tmp/tmpproxy.$$
rm -f /tmp/tmpproxy.$$
myexit 1 "failed to verify certificate from: $certfile and $keyfile\n " 
fi
return 0
}
