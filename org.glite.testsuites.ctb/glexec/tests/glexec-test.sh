#!/bin/sh


showUsage ()
{
 echo "                                           "
 echo "Usage:  $0 <proxy>    "
 echo "  <proxy> a VOMS proxy certificate"
 echo "                                           "
}


if [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$1" = "--help" ] || [ $# -gt 2 ] || [ -z $1 ];
 then
  showUsage
  exit 2
fi


hostname
echo ===
date
echo ===
target_proxy=$1

export GLEXEC_CLIENT_CERT=$target_proxy
export GLEXEC_SOURCE_PROXY=$target_proxy

export X509_USER_PROXY=$target_proxy

#$GLITE_LOCATION/sbin/glexec "/opt/glite/bin/grid-proxy-info"
$GLITE_LOCATION/sbin/glexec "/usr/bin/whoami"
echo "Glexec returned $?"

