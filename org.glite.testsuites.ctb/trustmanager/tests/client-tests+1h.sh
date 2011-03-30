#!/bin/sh

# These test test that the server webintegration part gets
# the correct information from the certificates.
# The test requires that the trustmanager-test webapp
# has been deployed on tomcat. 
# This assumes that the certificates were created with
# org.glite.security.test-utils

#config variables
#tomcat host and port
export HOST=localhost:8443
#tomcat host certificate
#tomcat webapp dir
export TOMCAT_WEBAPP=/var/lib/tomcat5/webapps/
export WEBAPPNAME=trustmanager-test
#end of config variables

SUCCESS=1
FAIL=0

function myexit() {
  
  if [ -f /etc/grid-security/certificates/$ca_hash.r0.bak  ]  ; then
   mv /etc/grid-security/certificates/$ca_hash.r0.bak /etc/grid-security/certificates/$ca_hash.r0
  fi

  if [ -f /etc/grid-security/certificates/$ca_hash.0.bak  ]  ; then
   mv /etc/grid-security/certificates/$ca_hash.0.bak /etc/grid-security/certificates/$ca_hash.0
  fi

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
  echo "#trustmanager client tests# $1"
}

usage() {
 echo
 echo "Test that the trustmanager client works, and connects correctly to a server"
 echo "This test assumes that you're using certificates by"
 echo "org.glite.security.test-utils and that you have compiled CallEchoService"
 echo "Usage:"
 echo "======"
 echo "client-tests.sh --certdir <directory for test-utils certs> "
 echo ""
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

if [ x"$certdir" == x ]  ; then
 usage
 exit 1
fi
certdir=$(readlink -f $certdir)

ca_hash=`openssl x509 -in $certdir/trusted-ca/trusted.cert -noout -hash`

#Check that the CRL has expired
exp=`openssl crl -in /etc/grid-security/certificates/$ca_hash.r0 -noout -nextupdate | cut -f2 -d = `
whene=`date -d "$exp" +%s`
now=`date +%s`

if [ $now -lt $whene ] ; then
 echo "CRL not yet expired, CRL will expire at $exp"
 exit 0
fi


if [ ! -d "$TOMCAT_WEBAPP/$WEBAPPNAME" ] ; then
 my_echo "Could not find the trustmanager webapp in $TOMCAT_WEBAPP/$WEBAPPNAME"
 my_exit 1
fi

CLASSPATH=$TOMCAT_WEBAPP/$WEBAPPNAME/WEB-INF/classes

CLASSPATH=/usr/share/java/commons-logging.jar:/usr/share/java/commons-discovery.jar:$CLASSPATH

for f in $TOMCAT_WEBAPP/$WEBAPPNAME/WEB-INF/lib/*.jar  ; do 
 CLASSPATH=$CLASSPATH:$f
done

export CLASSPATH
echo classpath is: $CLASSPATH

cd $TOMCAT_WEBAPP/$WEBAPPNAME/WEB-INF/classes

myecho "Testing client against expired CRL" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/$WEBAPPNAME/services/EchoService  |grep "CRL has expired"

if [ $? -ne 0 ] ; then 
 myecho "Succesfully connected to service even if the CRL was expired."  
 myexit 1
fi

myecho "Connection correctly failed when testing against an expired CRL"

myecho "Removing the CRL file"

mv /etc/grid-security/certificates/$ca_hash.r0 /etc/grid-security/certificates/$ca_hash.r0.bak 
if [ $? -ne 0 ] ; then 
    myecho "deleting of the CRL of CA $ca_hash failed."
    myexit 1
fi

myecho "Testing client against CA without CRL" 
CMD="java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/$WEBAPPNAME/services/EchoService"
$CMD  |grep "no CRL was found"

if [ $? -ne 0 ] ; then 
 myecho "Succesfully connected to service even if the CRL of CA $ca_hash was missing."  
 myecho "command was:"
 echo $CMD
 myexit 1
fi

myecho "Connection correctly failed when testing against an expired CRL"

myecho "Removing the CA file"

mv /etc/grid-security/certificates/$ca_hash.0 /etc/grid-security/certificates/$ca_hash.0.bak 

myecho "Testing client against untrusted CA" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/$WEBAPPNAME/services/EchoService  |grep "peer not authenticated"

if [ $? -ne 0 ] ; then 
 myecho "Succesfully connected to service even if the CA was untrusted."  
 myexit 1
fi

myecho "Connection correctly failed when testing against an untrusted CA"

myexit 0
