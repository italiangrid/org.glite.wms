#!/bin/sh

# These test test that the server web integration part gets
# the correct information from the certificates.
# The test requires that the trustmanager-test webapp
# has been deployed on tomcat. 
# This assumes that the certificates were created with
# org.glite.security.test-utils

#config variables
#tomcat host and port
export HOST=`hostname -f`:8443
#tomcat host certificate
export TOMCAT_CERT=/etc/grid-security/tomcat-cert.pem
#tomcat host key
export TOMCAT_KEY=/etc/grid-security/tomcat-key.pem
#tomcat service name (for restart)
rpm -qa |grep tomcat5
RES=$?
if [ $RES = 0 ]; then
    export TOMCAT_SERVICE=tomcat5
else
    export TOMCAT_SERVICE=tomcat6
fi

#tomcat webapp dir
export TOMCAT_WEBAPP=/var/lib/${TOMCAT_SERVICE}/webapps/
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
   /sbin/service $TOMCAT_SERVICE restart
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

if [ ! -d "$TOMCAT_WEBAPP/trustmanager-test" ] ; then
 my_echo "Could not find the trustmanager webapp in $TOMCAT_WEBAPP/trustmanager-test"
 my_exit 1
fi

CLASSPATH=$TOMCAT_WEBAPP/trustmanager-test/WEB-INF/classes


for f in $TOMCAT_WEBAPP/trustmanager-test/WEB-INF/lib/*.jar  ; do 
 CLASSPATH=$CLASSPATH:$f
done

CLASSPATH=/usr/share/java/commons-logging.jar:/usr/share/java/commons-discovery.jar:$CLASSPATH

export CLASSPATH
echo classpath is: $CLASSPATH

pushd .
cd $TOMCAT_WEBAPP/trustmanager-test/WEB-INF/classes

myecho "Testing client with normal certificate" 
CMD="java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/trustmanager-test/services/EchoService"
$CMD |grep EchoSecurityService
if [ $? -ne 0 ] ; then 
 myecho "Error connecting to service with a normal client certificate. Please change to $clidir and run the following to see the specific errors:"  
 echo $CMD 
 myexit 1
fi

myecho "Test with normal certificate successful"

myecho "Testing client with proxy certificate"
CMD="java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates  -DgridProxyFile=$certdir/trusted-certs/trusted_client.proxy.grid_proxy org/glite/security/trustmanager/axis/CallEchoService https://$HOST/trustmanager-test/services/EchoService"
$CMD |grep EchoSecurityService

if [ $? -ne 0 ] ; then 
 myecho "Error connecting to service with a proxy client certificate. Please change to $clidir and run the following to see the specific errors:"  
 echo $CMD
 myexit 1
fi

myecho "Testing client with pkcs8 key (bug #69163)"
CMD="java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client.priv.pkcs8 org/glite/security/trustmanager/axis/CallEchoService https://$HOST/trustmanager-test/services/EchoService"
$CMD |grep EchoSecurityService

if [ $? -ne 0 ] ; then 
 myecho "Error connecting to service with a pkcs8 key. Please change to $clidir and run the following to see the specific errors:"  
 echo $CMD
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
/sbin/service $TOMCAT_SERVICE restart
sleep 15

myecho "Confirming that tomcat came up properly"
wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv https://$HOST/trustmanager-test/servlet/EchoSecurity -O /dev/null

if [ $? -ne 0 ] ; then 
 myecho "Tomcat didn't seem to come up properly. Please check tomcat logs. Is this bug #56623, or did you just forget to add internalOverrideExpirationCheck=\"true\" in tomcat's server.xml?"
 myexit 1
fi

myecho "Tomcat up and running"


myecho "Testing against expired host certificate" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/trustmanager-test/services/EchoService | grep expired 

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
/sbin/service $TOMCAT_SERVICE restart
sleep 15

myecho "Confirming that tomcat came up properly"
wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv https://$HOST/trustmanager-test/servlet/EchoSecurity -O /dev/null

if [ $? -ne 0 ] ; then 
 myecho "Tomcat didn't seem to come up properly. Please check tomcat logs"
 myexit 1
fi

myecho "Tomcat up and running"

myecho "Testing against revoked host certificate" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/trustmanager-test/services/EchoService | grep revoked

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
/sbin/service $TOMCAT_SERVICE restart
sleep 15

myecho "Confirming that tomcat came up properly"
wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv https://$HOST/trustmanager-test/servlet/EchoSecurity -O /dev/null

if [ $? -ne 0 ] ; then 
 myecho "Tomcat didn't seem to come up properly. Please check tomcat logs"
 myexit 1
fi

myecho "Tomcat up and running"

myecho "Testing against host certificate not conforming to the namespace" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/trustmanager-test/services/EchoService | grep "peer not authenticated"

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
/sbin/service $TOMCAT_SERVICE restart
sleep 15

myecho "Confirming that tomcat came up properly"
wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv https://$HOST/trustmanager-test/servlet/EchoSecurity -O /dev/null

if [ $? -ne 0 ] ; then 
 myecho "Tomcat didn't seem to come up properly. Please check tomcat logs"
 myexit 1
fi

myecho "Tomcat up and running"

myecho "Testing against host certificate with an emailAddress field (bug#69449)" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/trustmanager-test/services/EchoService | grep EchoSecurityService

if [ $? -ne 0 ] ; then 
 myecho "Connection to server failed, when it should have succeeded"
 myexit 1
fi

myecho "Test against host certificate with an emailAddress field successful"



myecho "Switching tomcat certificate with bad CN (trusted/ prefix) and altname"


cp -f $certdir/trusted-certs/trusted_altname.cert $TOMCAT_CERT
cp -f $certdir/trusted-certs/trusted_altname_nopass.priv $TOMCAT_KEY

chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_CERT
chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_KEY

myecho "Restarting tomcat"
/sbin/service $TOMCAT_SERVICE restart
sleep 15

myecho "Confirming that tomcat came up properly"
wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv https://$HOST/trustmanager-test/servlet/EchoSecurity -O /dev/null

if [ $? -ne 0 ] ; then 
 myecho "Tomcat didn't seem to come up properly. Please check tomcat logs. Is this bug #56623, or did you just forget to add internalOverrideExpirationCheck=\"true\" in tomcat's server.xml?"
 myexit 1
fi

myecho "Tomcat up and running"


myecho "Testing against bad CN (trusted/ prefix) and altname" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/trustmanager-test/services/EchoService  |grep "not allowed with certificate for DN"

if [ $? -ne 0 ] ; then 
 myecho "Connection to server succesful even if server certificate has bad CN and altname"
 myexit 1
fi


myecho "Test against host certificate with a bad CN (trusted/ prefix) and altname succesful"

myecho "Switching tomcat certificate with bad CN and altname"


cp -f $certdir/trusted-certs/trusted_altname_2.cert $TOMCAT_CERT
cp -f $certdir/trusted-certs/trusted_altname_2_nopass.priv $TOMCAT_KEY

chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_CERT
chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_KEY

myecho "Restarting tomcat"
/sbin/service $TOMCAT_SERVICE restart
sleep 15

myecho "Confirming that tomcat came up properly"
wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv https://$HOST/trustmanager-test/servlet/EchoSecurity -O /dev/null

if [ $? -ne 0 ] ; then 
 myecho "Tomcat didn't seem to come up properly. Please check tomcat logs. Is this bug #56623, or did you just forget to add internalOverrideExpirationCheck=\"true\" in tomcat's server.xml?"
 myexit 1
fi

myecho "Tomcat up and running"

myecho "Testing against bad CN and altname" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/trustmanager-test/services/EchoService |grep "not allowed with certificate for DN"

if [ $? -ne 0 ] ; then 
 myecho "Connection to server succesful even if server certificate has bad CN and altname"
 myexit 1
fi


myecho "Test against host certificate with a bad CN and altname succesful"

myecho "Switching tomcat certificate to one with hostname only in the altname"


cp -f $certdir/trusted-certs/trusted_altname2.cert $TOMCAT_CERT
cp -f $certdir/trusted-certs/trusted_altname2_nopass.priv $TOMCAT_KEY

chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_CERT
chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_KEY

myecho "Restarting tomcat"
/sbin/service $TOMCAT_SERVICE restart
sleep 15

myecho "Confirming that tomcat came up properly"
wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv https://$HOST/trustmanager-test/servlet/EchoSecurity -O /dev/null

if [ $? -ne 0 ] ; then 
 myecho "Tomcat didn't seem to come up properly. Please check tomcat logs. Is this bug #56623, or did you just forget to add internalOverrideExpirationCheck=\"true\" in tomcat's server.xml?"
 myexit 1
fi

myecho "Tomcat up and running"


myecho "Testing against certificate with hostname only in the altname" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/trustmanager-test/services/EchoService |grep EchoSecurityService

if [ $? -ne 0 ] ; then 
 myecho "Connection to server failed even if altname had a correct hostname"
 myexit 1
fi


myecho "Test against host certificate with hostname only in altname succesful"

myecho "Switching tomcat certificate to one with hostname in the altname and CN"


cp -f $certdir/trusted-certs/trusted_altname2_2.cert $TOMCAT_CERT
cp -f $certdir/trusted-certs/trusted_altname2_2_nopass.priv $TOMCAT_KEY

chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_CERT
chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_KEY

myecho "Restarting tomcat"
/sbin/service $TOMCAT_SERVICE restart
sleep 15

myecho "Confirming that tomcat came up properly"
wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv https://$HOST/trustmanager-test/servlet/EchoSecurity -O /dev/null

if [ $? -ne 0 ] ; then 
 myecho "Tomcat didn't seem to come up properly. Please check tomcat logs. Is this bug #56623, or did you just forget to add internalOverrideExpirationCheck=\"true\" in tomcat's server.xml?"
 myexit 1
fi

myecho "Tomcat up and running"


myecho "Testing against certificate with hostname in the CN and altname" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/trustmanager-test/services/EchoService |grep EchoSecurityService

if [ $? -ne 0 ] ; then 
 myecho "Connection to server failed even if CN and altname had a correct hostname"
 myexit 1
fi


myecho "Test against host certificate with hostname in CN and altname succesful"

myecho "Switching tomcat certificate to one with an email address in altname and no hostnames"


cp -f $certdir/trusted-certs/trusted_altname3.cert $TOMCAT_CERT
cp -f $certdir/trusted-certs/trusted_altname3_nopass.priv $TOMCAT_KEY

chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_CERT
chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_KEY

myecho "Restarting tomcat"
/sbin/service $TOMCAT_SERVICE restart
sleep 15

myecho "Confirming that tomcat came up properly"
wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv https://$HOST/trustmanager-test/servlet/EchoSecurity -O /dev/null

if [ $? -ne 0 ] ; then 
 myecho "Tomcat didn't seem to come up properly. Please check tomcat logs. Is this bug #56623, or did you just forget to add internalOverrideExpirationCheck=\"true\" in tomcat's server.xml?"
 myexit 1
fi

myecho "Tomcat up and running"


myecho "Testing against certificate with email address in altname and no hostnames" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/trustmanager-test/services/EchoService |grep "not allowed with certificate for DN"

if [ $? -ne 0 ] ; then 
 myecho "Connection to server succesful even if there were no hostnames anywhere"
 myexit 1
fi


myecho "Test against host certificate with email address in altname and no hostnames succesful"

myecho "Switching tomcat certificate to one with an email address in altname and hostname in CN"


cp -f $certdir/trusted-certs/trusted_altname3_2.cert $TOMCAT_CERT
cp -f $certdir/trusted-certs/trusted_altname3_2_nopass.priv $TOMCAT_KEY

chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_CERT
chown $TOMCAT_CERT_OWN:$TOMCAT_CERT_GRP $TOMCAT_KEY

myecho "Restarting tomcat"
/sbin/service $TOMCAT_SERVICE restart
sleep 15

myecho "Confirming that tomcat came up properly"
wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv https://$HOST/trustmanager-test/servlet/EchoSecurity -O /dev/null

if [ $? -ne 0 ] ; then 
 myecho "Tomcat didn't seem to come up properly. Please check tomcat logs. Is this bug #56623, or did you just forget to add internalOverrideExpirationCheck=\"true\" in tomcat's server.xml?"
 myexit 1
fi

myecho "Tomcat up and running"


myecho "Testing against certificate with email address in altname and hostnames in CN" 
java  -Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DtrustStoreDir=/etc/grid-security/certificates -DsslCertFile=$certdir/trusted-certs/trusted_client.cert -DsslKey=$certdir/trusted-certs/trusted_client_nopass.priv org/glite/security/trustmanager/axis/CallEchoService https://$HOST/trustmanager-test/services/EchoService |grep EchoSecurityService

if [ $? -ne 0 ] ; then 
 myecho "Connection to server failed even if the hostname was in the CN"
 myexit 1
fi


myecho "Test against host certificate with email address in altname and hostname in the CN succesful"


myexit 0
