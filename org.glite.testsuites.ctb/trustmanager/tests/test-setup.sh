#!/bin/bash

# This scirpt sets up a proper environment for the tests. This mainly consists of copying around certificates..

usage() {
 echo
 echo "Script for setting up the trustmanager test environment (copying around certs)."
 echo "You need to have created the certificates already with org.glite.security.test-utils"
 echo
 echo "test-setup.py --certdir <directory for test-utils certs>"
 echo
}

function removePassPhrase ()  {
 openssl rsa  -passin pass:changeit -in $certdir/$1.priv -out $certdir/$1_nopass.priv
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

echo "Copying host certificates"

cp $certdir/grid-security/host* /etc/grid-security/

if [ $? -ne 0 ] ; then
 echo "Error copying certificates"
 exit 1
fi

echo "Generating short crl"
cd $certdir/trusted-ca
export CA_DIR=.
export CASROOT=$certdir
export CATYPE=trusted

openssl ca -gencrl -crlhours 1 -out $CA_DIR/trusted.crl -config $CA_DIR/req_conf.cnf

cd -

echo "Copying CA certificates"
for ca in expired trusted ; do
 ca_hash=`openssl x509 -in $certdir/$ca-ca/$ca.cert -noout -hash`
 cp $certdir/$ca-ca/$ca.cert /etc/grid-security/certificates/$ca_hash.0
 cp $certdir/$ca-ca/$ca.crl /etc/grid-security/certificates/$ca_hash.r0
 if [ -f $certdir/$ca-ca/$ca.namespaces ] ; then 
  cp $certdir/$ca-ca/$ca.namespaces /etc/grid-security/certificates/$ca_hash.namespaces
 fi
 if [ -f $certdir/$ca-ca/$ca.signing_policy ] ; then 
  cp $certdir/$ca-ca/$ca.signing_policy /etc/grid-security/certificates/$ca_hash.signing_policy
 fi

done

echo "Removing passphrases from certificates"
while read LINE; do 
 removePassPhrase $LINE 
done < testcerts.txt

chmod 400 $certdir/trusted-certs/*priv
echo "Creating a legacy proxy certificate"
voms-proxy-init -proxyver old -key $certdir/trusted-certs/trusted_client_nopass.priv -cert $certdir/trusted-certs/trusted_client.cert -out $certdir/trusted-certs/trusted_client.proxy.legacy

echo "Creating a rfc proxy certificate"
voms-proxy-init -rfc  -key $certdir/trusted-certs/trusted_client_nopass.priv -cert $certdir/trusted-certs/trusted_client.cert -out $certdir/trusted-certs/trusted_client.proxy.rfc


echo "Creating a correct trust chain for proxy proxies"
cat $certdir/trusted-certs/trusted_client.cert $certdir/trusted-certs/trusted_client.proxy.cert > $certdir/trusted-certs/trusted_client_proxy_chain




echo "Copying done, please restart tomcat"
