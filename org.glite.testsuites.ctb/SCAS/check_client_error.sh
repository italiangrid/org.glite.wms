#!/bin/sh

function usage() {

echo "Usage $0 <user number>"
echo "  <user number> Number of the test user"
}

if [ $# -ne 1 ];then
  echo  "Wrong arguments"
  usage
  exit 1
else
  usernum=$1
fi

fail=0
export GLITE_LOCATION=/opt/glite

function createProxy() {

keyfile="test_user_${usernum}_key.pem"
certfile="test_user_${usernum}_cert.pem"
proxy="x509up_u501_${usernum}"
if [ ! -f $keyfile ] || [ ! -f $certfile ]; then
  echo "test user certificate r key not found"
  echo "Test 2 failed"
  fail=1
fi

echo "Creating proxy file x509up_u501_${usernum}"  
echo "test" | glite-voms-proxy-init -q --voms dteam -cert ./test_user_${usernum}_cert.pem -key test_user_${usernum}_key.pem -out ./x509up_u501_${usernum} -pwstdin
if [ $? -ne 0 ]; then
  echo "Error creating the proxy" 
  exit 1
else
  echo "Proxy created"
fi
}

createProxy
proxy="x509up_u501_${usernum}"

echo "Check that glexec works with correct settings"
export GLEXEC_CLIENT_CERT=$proxy
export GLEXEC_SOURCE_PROXY=$proxy
export X509_USER_PROXY=$proxy
$GLITE_LOCATION/sbin/glexec "/usr/bin/whoami"
ret=$?
if [ $ret -ne 0 ]; then
  echo "Return code expected was 0 but $ret was given"
  echo "Pre-testfailed (proxy was $proxy)"
  exit 1
else
  echo "Pre-test OK"
fi

##################################################################
echo "Test 1:"
echo "Check that glexec returns 201 when no proxy is provided"
rm -f $proxy
unset GLEXEC_CLIENT_CERT
unset GLEXEC_SOURCE_PROXY
unset X509_USER_PROXY
$GLITE_LOCATION/sbin/glexec "/usr/bin/whoami"
ret=$?
if [ $ret -ne 201 ]; then
  echo "Return code expected was 201 but $ret was given"
  echo "Test 1 failed (Know Issue)"
  fail=1
fi
##################################################################

createProxy

##################################################################
echo "Test 2:"
echo "Check that glexec returns 201 with wrong proxy permissions"

chmod 666 $proxy
GLEXEC_CLIENT_CERT=$proxy
GLEXEC_SOURCE_PROXY=$proxy
X509_USER_PROXY=$proxy

$GLITE_LOCATION/sbin/glexec "/usr/bin/whoami"
ret=$?
if [ $ret -ne 201 ]; then
  echo "Return code expected was 201 but $ret was given"
  echo "Test 2 failed (known issue)"
  fail=1
else
  echo "Test 2 passed"
fi

chmod 600 $proxy

##################################################################

echo "Test 3:"
echo "Check that glexec returns 201 when target location is not accessible"

export GLEXEC_CLIENT_CERT=$proxy
export GLEXEC_SOURCE_PROXY=$proxy
export X509_USER_PROXY=$proxy
export GLEXEC_TARGET_PROXY="/root"

$GLITE_LOCATION/sbin/glexec "/usr/bin/whoami"
ret=$?
if [ $ret -ne 201 ]; then
  echo "Return code expected was 201 but $ret was given"
  echo "Test 3 failed"
  fail=1
else
  echo "Test 3 passed"
fi



##################################################################
#echo "Test 10:"
#echo "Check that glexec returns 201 when user proxy is expired"

#keyfile="test_user_${usernum}_key.pem"
#certfile="test_user_${usernum}_cert.pem"
#proxy="x509up_u501_${usernum}"
#if [ ! -f $keyfile ] || [ ! -f $certfile ]; then
#  echo "test user certificate r key not found"
#  echo "Test 2 failed"
#  fail=1
#fi

#echo "Creating proxy file x509up_u501_${usernum}"  
#echo "test" | glite-voms-proxy-init -valid 0:1 -q --voms dteam -cert ./test_user_${usernum}_cert.pem -key test_user_${usernum}_key.pem -out ./x509up_u501_${usernum} -pwstdin
#if [ $? -ne 0 ]; then
#  echo "Error creating the proxy" 
#  fail=1
#else
#  echo "Sleep 2 minutes to let the proxy expires"
#  sleep 2m
#  export GLEXEC_CLIENT_CERT=$proxy
#  export GLEXEC_SOURCE_PROXY=$proxy
#  export  X509_USER_PROXY=$proxy
#  $GLITE_LOCATION/sbin/glexec "/usr/bin/whoami"
#  ret=$?
#  if [ $ret -ne 201 ]; then
#    echo "Return code expected was 201 but $ret was given"
#    echo "Test 2 failed"
#    fail=1
#  else
#    echo "Test 2 passed"
#  fi
#fi
##################################################################


if [ $fail -eq 0 ];then
  echo "TEST PASSED"
  exit 0
else
  echo "TEST FAILED"
  exit 1
fi

exit 0
