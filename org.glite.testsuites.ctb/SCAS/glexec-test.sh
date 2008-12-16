#!/bin/sh

[[ -z $1 ]] && echo "No Proxy specified" && exit 1

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

echo ===
date
