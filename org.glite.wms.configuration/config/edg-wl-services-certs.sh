if [ -f /etc/sysconfig/edg ]; then
   . /etc/sysconfig/edg
fi

if [ -z "$X509_USER_CERT" ]; then
  export X509_HOST_CERT=/etc/grid-security/hostcert.pem
else
  export X509_HOST_CERT=$X509_USER_CERT
fi

if [ -z "$X509_USER_KEY" ]; then
  export X509_HOST_KEY=/etc/grid-security/hostkey.pem
else
  export X509_HOST_KEY=$X509_USER_KEY
fi

