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
  exit 1
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
  fail=`echo "$fail +1" | bc`
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
  fail=`echo "$fail +1" | bc`
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
  fail=`echo "$fail +1" | bc`
else
  echo "Test 3 passed"
fi

##################################################################

echo "Test 4:"
echo "Check that glexec returns 201 when the binary to execute does not exist"

export GLEXEC_CLIENT_CERT=$proxy
export GLEXEC_SOURCE_PROXY=$proxy
export X509_USER_PROXY=$proxy
unset GLEXEC_TARGET_PROXY

$GLITE_LOCATION/sbin/glexec "/usr/bin/idontexist"
ret=$?
if [ $ret -ne 201 ]; then
  echo "Return code expected was 201 but $ret was given"
  echo "Test 4 failed"
  fail=`echo "$fail +1" | bc`
else
  echo "Test 4 passed"
fi

##################################################################

echo "Test 5:"
echo "Check that glexec returns 201 when the mapped user has no rigths to execute the binary"

export GLEXEC_CLIENT_CERT=$proxy
export GLEXEC_SOURCE_PROXY=$proxy
export X509_USER_PROXY=$proxy
unset GLEXEC_TARGET_PROXY

$GLITE_LOCATION/sbin/glexec "/tmp/mytest.sh"
ret=$?
if [ $ret -ne 201 ]; then
  echo "Return code expected was 201 but $ret was given"
  echo "Test 5 failed"
  fail=`echo "$fail +1" | bc`
else
  echo "Test 5 passed"
fi

##################################################################

echo "Test 6:"
echo "Check that glexec returns 201 when GLEXEC_CLIENT_CERT is not set"

unset GLEXEC_CLIENT_CERT
export GLEXEC_SOURCE_PROXY=$proxy
export X509_USER_PROXY=$proxy
unset GLEXEC_TARGET_PROXY

$GLITE_LOCATION/sbin/glexec "/usr/bin/whoami"
ret=$?
if [ $ret -ne 201 ]; then
  echo "Return code expected was 201 but $ret was given"
  echo "Test 6 failed"
  fail=`echo "$fail +1" | bc`
else
  echo "Test 6 passed"
fi

#################################################################


if [ $fail -eq 0 ];then
  echo "TEST PASSED"
  exit 0
else
  echo "$fail TESTS FAILED"
  exit 1
fi

exit 0
