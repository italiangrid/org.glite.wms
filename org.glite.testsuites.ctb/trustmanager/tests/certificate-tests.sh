#!/bin/sh

# Test connecting with a number of different certificates
# This assumes that the certificates were created with
# org.glite.security.test-utils

#config variables
#tomcat host and port
export HOST=localhost:8443
#end of config variables

SUCCESS=1
FAIL=0

function myexit() {

  if [ $1 -ne 0 ]; then
    echo " *** something went wrong *** "
    echo " *** test NOT passed *** "
    exit $1
  else
    echo ""
    echo "    === test PASSED === "
  fi
   
  exit 0
}

function myecho()
{
  echo "#trustmanager certificate tests# $1"
}

usage() {
 echo
 echo "Test different certifcates against trustmanager"
 echo "This test assumes that you're using certificates"
 echo "by org.glite.security.test-utils"
 echo "Usage:"
 echo "======"
 echo "certificate-tests.sh --certdir <directory for test-utils certs>"
 echo ""
}

function test_cert() {
 KEY=$1
 CERT=$2
 OUTCOME=$3

 if [ x"$4" != x ] ;  then 
  CA_CMD="-CAfile $4"
 fi

 openssl s_client -key $KEY -cert $CERT -CApath /etc/grid-security/certificates $CA_CMD -connect $HOST < input.txt  2>/dev/null |grep "(ok)" 
 RES=$?

 if [ $OUTCOME -eq $SUCCESS ] ; then 
  if [ $RES -ne 0 ] ; then
   myecho "Error, testing with $CERT failed when it should have suceeded"
   myexit 1
  fi
 else
  if [ $RES -eq 0 ] ; then
   myecho "Error, testing with $CERT succeeded when it should have failed"
   myexit 1
  fi
 fi
 
}

while [ $# -gt 0 ]
do
 case $1 in
 --certdir | -c ) certdir=$2
  shift
  ;;
 --help | -help | --h | -h ) usage
  exit 0
  ;;
 --* | -* ) echo "$0: invalid option $1" >&2
  usage
  exit 1
  ;;
 *) break
  ;;
 esac
 shift
done

if [ x"$certdir" == x ] ; then
 usage
 exit 1
fi

myecho "Testing with normal certificate"
test_cert $certdir/trusted-certs/trusted_client_nopass.priv $certdir/trusted-certs/trusted_client.cert $SUCCESS 
myecho "Test passed"
myecho "Testing with normal proxy certificate"
test_cert $certdir/trusted-certs/trusted_client.proxy_nopass.priv $certdir/trusted-certs/trusted_client.proxy.cert $SUCCESS $certdir/trusted-certs/trusted_client.cert
myecho "Test passed"
myecho "Testing with proxy proxy certificate"
test_cert $certdir/trusted-certs/trusted_client.proxy_proxy.proxy_nopass.priv $certdir/trusted-certs/trusted_client.proxy_proxy.proxy.cert $SUCCESS $certdir/trusted-certs/trusted_client_proxy_chain
myecho "Test passed"
myecho "Testing with legacy proxy certificate"
test_cert $certdir/trusted-certs/trusted_client.proxy.legacy  $certdir/trusted-certs/trusted_client.proxy.legacy $SUCCESS $certdir/trusted-certs/trusted_client.cert
myecho "Test passed"
myecho "Testing with rfc proxy certificate"
test_cert $certdir/trusted-certs/trusted_client.proxy.rfc  $certdir/trusted-certs/trusted_client.proxy.rfc $SUCCESS $certdir/trusted-certs/trusted_client.cert
myecho "Test passed"
myecho "Testing with a proxy certificate with false dn"
test_cert $certdir/trusted-certs/trusted_client.proxy_dnerror_nopass.priv $certdir/trusted-certs/trusted_client.proxy_dnerror.cert $FAIL $certdir/trusted-certs/trusted_client.cert
myecho "Test passed"
myecho "Testing with expired proxy certificate"
test_cert $certdir/trusted-certs/trusted_client.proxy_exp_nopass.priv $certdir/trusted-certs/trusted_client.proxy_exp.cert $FAIL $certdir/trusted-certs/trusted_client.cert
myecho "Test passed"
myecho "Testing with expired certificate"
test_cert $certdir/trusted-certs/trusted_client_exp_nopass.priv $certdir/trusted-certs/trusted_client_exp.cert $FAIL 
myecho "Test passed"
myecho "Testing with proxy of expired certificate"
test_cert $certdir/trusted-certs/trusted_client_exp.proxy_nopass.priv $certdir/trusted-certs/trusted_client_exp.proxy.cert $FAIL $certdir/trusted-certs/trusted_client_exp.cert
myecho "Test passed"
myecho "Testing with revoked certificate"
test_cert $certdir/trusted-certs/trusted_client_rev_nopass.priv $certdir/trusted-certs/trusted_client_rev.cert $FAIL 
myecho "Test passed"
myecho "Testing with proxy of revoked certificate"
test_cert $certdir/trusted-certs/trusted_client_rev.proxy_nopass.priv $certdir/trusted-certs/trusted_client_rev.proxy.cert $FAIL $certdir/trusted-certs/trusted_client_rev.cert
myecho "Test passed"
myecho "Testing with certificate from expired CA"
test_cert $certdir/expired-certs/expired_client_nopass.priv $certdir/expired-certs/expired_client.cert $FAIL 
myecho "Test passed"
myecho "Testing with proxy of certificate from expired CA"
test_cert $certdir/expired-certs/expired_client.proxy_nopass.priv $certdir/expired-certs/expired_client.proxy.cert $FAIL $certdir/expired-certs/expired_client.cert
myecho "Test passed"
myecho "Testing with untrusted certificate"
test_cert $certdir/fake-certs/fake_client_nopass.priv $certdir/fake-certs/fake_client.cert $FAIL
myecho "Test passed"
myecho "Testing with voms proxy certificate"
test_cert $certdir/home/voms-acme.pem $certdir/home/voms-acme.pem $SUCCESS $certdir/home/usercert.pem
myecho "Test passed"
myecho "Testing with voms proxy certificate with group"
test_cert $certdir/home/voms-acme-Gproduction.pem $certdir/home/voms-acme-Gproduction.pem $SUCCESS $certdir/home/usercert.pem
myecho "Test passed"
myecho "Testing with voms proxy certificate with role"
test_cert $certdir/home/voms-acme-Radmin.pem $certdir/home/voms-acme-Radmin.pem $SUCCESS $certdir/home/usercert.pem
myecho "Test passed"
myecho "Testing with the \"bad-ca\", a normal certificate"
test_cert $certdir/bad-certs/bad_client00_nopass.priv $certdir/bad-certs/bad_client00.cert  $SUCCESS 
myecho "Test passed"
myecho "Testing with the \"bad-ca\", a not yet valid certificate"
test_cert $certdir/bad-certs/bad_future_nopass.priv $certdir/bad-certs/bad_future.cert  $FAIL 
myecho "Test passed"
myecho "Testing with the \"bad-ca\", a certificate that doesn't match the signing policy nor namespace"
test_cert $certdir/bad-certs/bad_policy_nopass.priv $certdir/bad-certs/bad_policy.cert  $FAIL 
myecho "Test passed"

echo ""
myecho "Please run the certificate-tests+1h.sh in an hour"

myexit 0
