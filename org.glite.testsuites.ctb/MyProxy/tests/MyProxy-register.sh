#!/bin/bash


SAME_OK=0
SAME_ERROR=1

timestamp=`date +%s`
nodeName=$1

if [ $# -ne 1 ]; then
  echo "Usage: $0 <MyProxy host>"
  exit 1
fi

proxy=`voms-proxy-info --path`
export X509_USER_CERT=$proxy
export X509_USER_KEY=$proxy

function exitonerror {
    [ "x$1" = "x1" ] && exit $SAME_ERROR
    return 0
}

echo "test env :"
echo "<pre>"
cat <<EOF
SAME_OK value is $SAME_OK
SAME_ERROR value is $SAME_ERROR
EOF
echo "</pre>"

echo "Checking if we can first reach $nodeName using ping"
result=0
echo "<pre>"
ping -c 1 $nodeName
result=$?
[ $result -ne 0 ] && echo "could not reach $nodeName - test cannot go further"
echo "</pre>"
exitonerror $result

echo "Checking if a proxy can be registered to $nodeName"
result=0
echo "<pre>"
set -x
echo 'testpass' |  myproxy-init -d -s $nodeName -c 1 -S
result=$?
set +x
echo "</pre>"

exitonerror $result

echo "Checking if a proxy can be deleted on $nodeName"
echo "<pre>"
set -x
myproxy-destroy -d -s $nodeName
result=$?
set +x
echo "</pre>"
exitonerror $result

# If we're here, everything is OK
echo "-TEST PASSED-"
exit $SAME_OK

