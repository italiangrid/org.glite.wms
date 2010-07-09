#!/bin/sh

# These test test that the server webintegration part gets
# the correct information from the certificates.
# The test requires that the glite-security-trustmanager webapp
# has been deployed on tomcat. 
# This assumes that the certificates were created with
# org.glite.security.test-utils

#config variables
#tomcat host and port
export HOST=$HOSTNAME:8443
#tomcat host certificate
export TOMCAT_CERT=/etc/grid-security/tomcat-cert.pem
#tomcat host key
export TOMCAT_KEY=/etc/grid-security/tomcat-key.pem
#tomcat service name (for restart)
export TOMCAT_SERVICE=tomcat5
#tomcat webapp dir
export TOMCAT_WEBAPP=/var/lib/tomcat5/webapps/
#end of config variables

SUCCESS=1
FAIL=0

function myexit() {

  if [ -f $TOMCAT_CERT.bak ] ; then
   echo "Moving back the tomcat certificate"
   mv $TOMCAT_CERT.bak $TOMCAT_CERT
   chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_CERT
  fi 

  if [ -f $TOMCAT_KEY.bak ] ; then
   echo "Moving back the tomcat key"
   mv $TOMCAT_KEY.bak $TOMCAT_KEY
   chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_KEY
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
 echo "client-tests.sh --certdir <directory for test-utils certs>"
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

if [ x"$certdir" == x ] ; then
 usage
 exit 1
fi
certdir=$(readlink -f $certdir)

TOMCAT_CERT_OWN=`stat -c %U $TOMCAT_CERT`
TOMCAT_CERT_GRP=`stat -c %G $TOMCAT_CERT`

if [ ! -d "$TOMCAT_WEBAPP/glite-security-trustmanager" ] ; then
 my_echo "Could not find the trustmanager webapp in $TOMCAT_WEBAPP/glite-security-trustmanager"
 my_exit 1
fi

CLASSPATH=$TOMCAT_WEBAPP/glite-security-trustmanager/WEB-INF/classes


for f in $TOMCAT_WEBAPP/glite-security-trustmanager/WEB-INF/lib/*.jar  ; do 
 CLASSPATH=$CLASSPATH:$f
done

CLASSPATH=/opt/glite/share/java/glite-security-util-java.jar:$CLASSPATH

export CLASSPATH

pushd .
cd $TOMCAT_WEBAPP/glite-security-trustmanager/WEB-INF/classes

myecho "Testing client with normal certificate" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/glite-security-trustmanager/services/EchoService  |grep EchoSecurityService

if [ $? -ne 0 ] ; then 
 myecho "Error connecting to service with a normal client certificate. Please change to $clidir and run the following to see the specific errors:"  
 echo "java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/glite-security-trustmanager/services/EchoService"
 myexit 1
fi

myecho "Test with normal certificate successful"

myecho "Testing client with proxy certificate"
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates  -DgridProxyFile=$certdir/trusted-certs/trusted_client.proxy.grid_proxy org/glite/security/trustmanager/axis/CallEchoService https://$HOST/glite-security-trustmanager/services/EchoService |grep EchoSecurityService

if [ $? -ne 0 ] ; then 
 myecho "Error connecting to service with a proxy client certificate. Please change to $clidir and run the following to see the specific errors:"  
 echo "java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates  -DgridProxyFile=$certdir/trusted-certs/trusted_client.proxy.grid_proxy org/glite/security/trustmanager/axis/CallEchoService https://$HOST/glite-security-trustmanager/services/EchoService"
 myexit 1
fi

myecho "Testing client with pkcs8 key (bug #69163)"
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client.priv.pkcs8 org/glite/security/trustmanager/axis/CallEchoService https://$HOST/glite-security-trustmanager/services/EchoService  |grep EchoSecurityService

if [ $? -ne 0 ] ; then 
 myecho "Error connecting to service with a pkcs8 key. Please change to $clidir and run the following to see the specific errors:"  
 echo "java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client.priv.pkcs8 org/glite/security/trustmanager/axis/CallEchoService https://$HOST/glite-security-trustmanager/services/EchoService  |grep EchoSecurityService"
 myexit 1
fi

myecho "Test with proxy certificate successful"

myecho "Adding empty CA to test for bug #56623"

touch /etc/grid-security/certificates/12345678.0

myecho "Switching tomcat certificate to an expired one"

cp -f $TOMCAT_CERT $TOMCAT_CERT.bak
cp -f $TOMCAT_KEY $TOMCAT_KEY.bak

cp -f $certdir/trusted-certs/trusted_host_exp.cert $TOMCAT_CERT
cp -f $certdir/trusted-certs/trusted_host_exp_nopass.priv $TOMCAT_KEY

chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_CERT
chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_KEY

myecho "Restarting tomcat"
service $TOMCAT_SERVICE restart
sleep 15

myecho "Confirming that tomcat came up properly"
wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv https://$HOST/glite-security-trustmanager/servlet/EchoSecurity -O /dev/null

if [ $? -ne 0 ] ; then 
 myecho "Tomcat didn't seem to come up properly. Please check tomcat logs. Is this bug #56623, or did you just forget to add internalOverrideExpirationCheck=\"true\" in tomcat's server.xml?"
 myexit 1
fi

myecho "Tomcat up and running"


myecho "Testing against expired host certificate" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/glite-security-trustmanager/services/EchoService | grep expired 

if [ $? -ne 0 ] ; then 
 myecho "Connection to server succesful even if server certificate has expired"
 myexit 1
fi

myecho "Test against expired host certificate successful"

myecho "Switching tomcat certificate to a revoked one"

cp -f $certdir/trusted-certs/trusted_host_rev.cert $TOMCAT_CERT
cp -f $certdir/trusted-certs/trusted_host_rev_nopass.priv $TOMCAT_KEY

chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_CERT
chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_KEY

myecho "Restarting tomcat"
service $TOMCAT_SERVICE restart
sleep 15

myecho "Confirming that tomcat came up properly"
wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv https://$HOST/glite-security-trustmanager/servlet/EchoSecurity -O /dev/null

if [ $? -ne 0 ] ; then 
 myecho "Tomcat didn't seem to come up properly. Please check tomcat logs"
 myexit 1
fi

myecho "Tomcat up and running"

myecho "Testing against revoked host certificate" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/glite-security-trustmanager/services/EchoService | grep revoked

if [ $? -ne 0 ] ; then 
 myecho "Connection to server succesful even if server certificate has been revoked"
 myexit 1
fi

myecho "Test against revoked host certificate successful"


myecho "Switching tomcat certificate to one not conforming to the namespace"

cp -f $certdir/trusted-certs/trusted_host_baddn.cert $TOMCAT_CERT
cp -f $certdir/trusted-certs/trusted_host_baddn_nopass.priv $TOMCAT_KEY

chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_CERT
chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_KEY

myecho "Restarting tomcat"
service $TOMCAT_SERVICE restart
sleep 15

myecho "Confirming that tomcat came up properly"
wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv https://$HOST/glite-security-trustmanager/servlet/EchoSecurity -O /dev/null

if [ $? -ne 0 ] ; then 
 myecho "Tomcat didn't seem to come up properly. Please check tomcat logs"
 myexit 1
fi

myecho "Tomcat up and running"

myecho "Testing against host certificate not conforming to the namespace" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/glite-security-trustmanager/services/EchoService | grep namespace

if [ $? -ne 0 ] ; then 
 myecho "Connection to server succesful even if server certificate does not conform to the namespace"
 myexit 1
fi

myecho "Test against host certificate not conforming to the namespace successful"

myecho "Switching tomcat certificate to one with the emailAddress field"

cp -f $certdir/trusted-certs/trusted_host_email.cert $TOMCAT_CERT
cp -f $certdir/trusted-certs/trusted_host_email_nopass.priv $TOMCAT_KEY

chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_CERT
chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_KEY

myecho "Restarting tomcat"
service $TOMCAT_SERVICE restart
sleep 15

myecho "Confirming that tomcat came up properly"
wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv https://$HOST/glite-security-trustmanager/servlet/EchoSecurity -O /dev/null

if [ $? -ne 0 ] ; then 
 myecho "Tomcat didn't seem to come up properly. Please check tomcat logs"
 myexit 1
fi

myecho "Tomcat up and running"

myecho "Testing against host certificate with an emailAddress field (bug#69449)" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/glite-security-trustmanager/services/EchoService | grep EchoSecurityService

if [ $? -ne 0 ] ; then 
 myecho "Connection to server failed, when it should have succeeded"
 myexit 1
fi

myecho "Test against host certificate with an emailAddress field successful"
myexit 0
