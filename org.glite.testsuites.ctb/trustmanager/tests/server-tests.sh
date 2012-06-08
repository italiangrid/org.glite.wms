#!/bin/sh

# These test test that the server webintegration part gets
# the correct information from the certificates.
# The test requires that the trustmanager-test webapp
# has been deployed on tomcat. 
# This assumes that the certificates were created with
# org.glite.security.test-utils

#config variables
#tomcat host and port
export HOST=`hostname -f`:8443
export WEBAPPNAME=trustmanager-test
#end of config variables

SUCCESS=1
FAIL=0

function myexit() {
  if [ x$getfile != x ] ; then
   if [ -f $getfile ] ; then
    rm $getfile
   fi
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
  echo "#trustmanager server tests# $1"
}

usage() {
 echo
 echo "Test that the trustmanager gets and reports the correct certificate information"
 echo "This test assumes that you're using certificates"
 echo "by org.glite.security.test-utils"
 echo "Usage:"
 echo "======"
 echo "server-tests.sh --certdir <directory for test-utils certs>"
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

getfile=`mktemp`

if [ $? -ne 0 ] ; then
 myecho "Error making temporary file"
 myexit 1
fi

rm $getfile

myecho "Checking that the server reports the correct information for a normal certificate"

echo "curl -v -s -S --cert $certdir/trusted-certs/trusted_client.cert --key $certdir/trusted-certs/trusted_client_nopass.priv --capath /etc/grid-security/certificates/ https://$HOST/$WEBAPPNAME/servlet/EchoSecurity >$getfile"
curl -v -s -S --cert $certdir/trusted-certs/trusted_client.cert --key $certdir/trusted-certs/trusted_client_nopass.priv --capath /etc/grid-security/certificates/ https://$HOST/$WEBAPPNAME/servlet/EchoSecurity >$getfile
#wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.cert --private-key $certdir/trusted-certs/trusted_client_nopass.priv  https://$HOST/$WEBAPPNAME/servlet/EchoSecurity -O  $getfile

if [ $? -ne 0 ] ; then
 myecho "Error getting certificate information from server"
 myexit 1
fi

SSDN=`grep DN $getfile | sed -e "s/[^:]*:\ //"`

CDN=`openssl x509 -in $certdir/trusted-certs/trusted_client.cert -noout -subject | sed -e "s/subject=\ \///" -e "s/\//,/g"`

if [ x"$CDN" != x"$SSDN" ] ; then
 myecho "The certificate DN does not match what the server reports"
 myexit 1
fi

myecho "Certificate DN matches what the server reports"

SSIS=`grep Issued $getfile | sed -e "s/[^:]*:\ //"`

CIS=`openssl x509 -in $certdir/trusted-certs/trusted_client.cert -noout -issuer | sed -e "s/issuer=\ \///" -e "s/\//,/g"`

if [ x"$CIS" != x"$SSIS" ] ; then
 myecho "The certificate issuer does not match what the server reports"
 myexit 1
fi

myecho "Certificate issuer matches what the server reports"

SSFN=`grep "final certificate" $getfile | sed -e "s/[^:]*:\ //"`

if [ x"$CDN" != x"$SSFN" ] ; then
 myecho "The final certificate subject does not match what the server reports"
 myexit 1
fi

myecho "Final certificate subject matches what the server reports"

SSEC=`grep "end cert" $getfile | sed -e "s/[^:]*:\ //"`

if [ "xend-user certificate" != x"$SSEC" ] ; then
 myecho "The certificate type does not match what the server reports"
 myexit 1
fi

myecho "The certificate type matches what the server reports"

rm $getfile

myecho "Checking that the server reports the correct information for a proxy certificate"

curl -v -s -S --cert $certdir/trusted-certs/trusted_client.proxy.cert --key $certdir/trusted-certs/trusted_client.proxy_nopass.priv --cacert $certdir/trusted-certs/trusted_client.cert --capath /etc/grid-security/certificates/ https://$HOST/$WEBAPPNAME/servlet/EchoSecurity >$getfile

#wget --no-check-certificate --certificate  $certdir/trusted-certs/trusted_client.proxy.cert --private-key $certdir/trusted-certs/trusted_client.proxy_nopass.priv --ca-certificate $certdir/trusted-certs/trusted_client.cert https://$HOST/$WEBAPPNAME/servlet/EchoSecurity -O  $getfile

if [ $? -ne 0 ] ; then
 myecho "Error getting proxy certificate information from server"
 myexit 1
fi

SSPDN=`grep DN $getfile | sed -e "s/[^:]*:\ //"`

PDN=`openssl x509 -in $certdir/trusted-certs/trusted_client.proxy.cert -noout -subject | sed -e "s/subject=\ \///" -e "s/\//,/g"`

if [ x"$PDN" != x"$SSPDN",CN=proxy ] ; then
 myecho "The proxy DN does not match what the server reports"
 myexit 1
fi

myecho "Proxy DN matches what the server reports"

SSPIS=`grep Issued $getfile | sed -e "s/[^:]*:\ //"`

PIS=`openssl x509 -in $certdir/trusted-certs/trusted_client.cert -noout -issuer | sed -e "s/issuer=\ \///" -e "s/\//,/g"`

if [ x"$PIS" != x"$SSPIS" ] ; then
 myecho "The proxy issuer does not match what the server reports"
 myexit 1
fi

myecho "Proxy issuer matches what the server reports"

SSPFN=`grep "final certificate" $getfile | sed -e "s/[^:]*:\ //"`

if [ x"$PDN" != x"$SSPFN" ] ; then
 myecho "The final proxy subject does not match what the server reports"
 myexit 1
fi

myecho "Final certificate subject matches what the server reports"

SSEC=`grep "end cert" $getfile | sed -e "s/[^:]*:\ //"`
if [ "xproxy certificate" != x"$SSEC" ] ; then
 myecho "The certificate type does not match what the server reports"
 myexit 1
fi

myecho "The certificate type matches what the server reports"
myexit 0
