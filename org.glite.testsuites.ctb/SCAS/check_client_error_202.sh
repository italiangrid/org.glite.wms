#!/bin/sh

#This test must be run as root

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
export PATH="$PATH:$GLITE_LOCATION/bin"

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
echo "Check that glexec returns 202 when glexec.conf is not readable"

export GLEXEC_CLIENT_CERT=$proxy
export GLEXEC_SOURCE_PROXY=$proxy
export X509_USER_PROXY=$proxy
unset GLEXEC_TARGET_PROXY

#move out glexec.conf
mv /opt/glite/etc/glexec.conf /opt/glite/etc/glexec.conf.moved

$GLITE_LOCATION/sbin/glexec "/usr/bin/whoami"
ret=$?
if [ $ret -ne 202 ]; then
  echo "Return code expected was 202 but $ret was given"
  echo "Test 1 failed"
  fail=`echo "$fail +1" | bc`
else
  echo "Test 1 passed"
fi

#restore glexec.conf
mv /opt/glite/etc/glexec.conf.moved /opt/glite/etc/glexec.conf

##################################################################

echo "Test 2:"
echo "Check that glexec returns 202 when lcas.db file is not readable"

export GLEXEC_CLIENT_CERT=$proxy
export GLEXEC_SOURCE_PROXY=$proxy
export X509_USER_PROXY=$proxy
unset GLEXEC_TARGET_PROXY

#move out lcas-glexec.db
mv  /opt/glite/etc/lcas/lcas-glexec.db /opt/glite/etc/lcas/lcas-glexec.db.moved

$GLITE_LOCATION/sbin/glexec "/usr/bin/whoami"
ret=$?
if [ $ret -ne 202 ]; then
  echo "Return code expected was 202 but $ret was given"
  echo "Test 2 failed"
  fail=`echo "$fail +1" | bc`
else
  echo "Test 2 passed"
fi

#restore lcas-glexec.db
mv  /opt/glite/etc/lcas/lcas-glexec.db.moved /opt/glite/etc/lcas/lcas-glexec.db

##################################################################

echo "Test 3:"
echo "Check that glexec returns 202 when lcas.db file is not readable"

export GLEXEC_CLIENT_CERT=$proxy
export GLEXEC_SOURCE_PROXY=$proxy
export X509_USER_PROXY=$proxy
unset GLEXEC_TARGET_PROXY

#move out lcmaps-glexec.db
mv  /opt/glite/etc/lcmaps/lcmaps-glexec.db /opt/glite/etc/lcmaps/lcmaps-glexec.db.moved

$GLITE_LOCATION/sbin/glexec "/usr/bin/whoami"
ret=$?
if [ $ret -ne 202 ]; then
  echo "Return code expected was 202 but $ret was given"
  echo "Test 3 failed"
  fail=`echo "$fail +1" | bc`
else
  echo "Test 3 passed"
fi

#restore lcmaps-glexec.db
mv  /opt/glite/etc/lcmaps/lcmaps-glexec.db.moved /opt/glite/etc/lcmaps/lcmaps-glexec.db

##################################################################

echo "Test 4:"
echo "Check that glexec returns 202 when glexec.conf is malformed"

export GLEXEC_CLIENT_CERT=$proxy
export GLEXEC_SOURCE_PROXY=$proxy
export X509_USER_PROXY=$proxy
unset GLEXEC_TARGET_PROXY

#modify glexec.conf
cp /opt/glite/etc/glexec.conf /opt/glite/etc/glexec.conf.orig
echo "[dummy]" >> /opt/glite/etc/glexec.conf

$GLITE_LOCATION/sbin/glexec "/usr/bin/whoami"
ret=$?
if [ $ret -ne 202 ]; then
  echo "Return code expected was 202 but $ret was given"
  echo "Test 4 failed"
  fail=`echo "$fail +1" | bc`
else
  echo "Test 4 passed"
fi

#restore glexec.conf
cp -f /opt/glite/etc/glexec.conf.orig /opt/glite/etc/glexec.conf

##################################################################

if [ $fail -eq 0 ];then
  echo "TEST PASSED"
  exit 0
else
  echo "$fail TESTS FAILED"
  exit 1
fi

exit 0
