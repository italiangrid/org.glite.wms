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

cp $certdir/grid-security/hostcert.pem /etc/grid-security/tomcat-cert.pem
if [ $? -ne 0 ] ; then
 echo "Error copying host certificate"
 exit 1
fi
cp $certdir/grid-security/hostkey.pem /etc/grid-security/tomcat-key.pem
if [ $? -ne 0 ] ; then
 echo "Error copying host key"
 exit 1
fi

chown tomcat:tomcat /etc/grid-security/tomcat*

if [ $? -ne 0 ] ; then
 echo "Error changing host credential permissions"
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
cp $certdir/grid-security/certificates/* /etc/grid-security/certificates/

#copy the generated crl
ca_hash=`openssl x509 -in $certdir/trusted-ca/trusted.cert -noout -hash`
cp $certdir/trusted-ca/trusted.crl /etc/grid-security/certificates/$ca_hash.r0


echo "Removing passphrases from certificates"
while read LINE; do 
 removePassPhrase $LINE 
done < testcerts.txt

chmod 400 $certdir/trusted-certs/*priv


echo "Creating a correct trust chain for proxy proxies"
cat $certdir/trusted-certs/trusted_client.cert $certdir/trusted-certs/trusted_client.proxy.cert > $certdir/trusted-certs/trusted_client_proxy_chain
cat $certdir/trusted-certs/trusted_client.cert $certdir/trusted-certs/trusted_client.proxy_rfc_plen.cert $certdir/trusted-certs/trusted_client.proxy_rfc_plen.proxy_rfc.cert > $certdir/trusted-certs/trusted_client_rfc_proxy_chain


echo "Copying trustmanager WAR to tomcat"
cp /usr/share/java/trustmanager-test.war /var/lib/tomcat5/webapps/

echo "Copying done, please restart tomcat"
