#!/bin/sh

#
# bug #35997: FTS cli: if an invalid value is passed with -s option, the default is used
#

export PATH="$PATH:/opt/glite/bin"

pushd /opt/glite/bin

for i in glite-transfer*; 
do 
  echo "$i -s httpj://foo"
  if [ "$i" == "glite-transfer-service-info" ]; then
    message=`./$i --version -s httpj://foo 2>&1`
  else
    message=`./$i -s httpj://foo 2>&1`
  fi
  echo $message | grep 'Failed to determine the endpoint: Wrongly formatted endpoint'
  if [ $? -ne 0 ]; then
    echo "BUG PRESENT"
    exit 1
  else
    echo "OK"
  fi
done

for i in glite-transfer*;
do 
  echo "$i  -s https://foo"; 
  if [ "$i" == "glite-transfer-service-info" ]; then
    message=`./$i --version -s https://foo 2>&1`
  else
    message=`./$i -s https://foo 2>&1`
  fi
  echo $message | grep 'could not open connection to'
  if [ $? -ne 0 ]; then
    echo "BUG PRESENT"
    exit 1
  else
    echo "OK"
  fi
done

for i in glite-transfer*; 
do 
  echo "$i  -s httpg://foo"; 
  if [ "$i" == "glite-transfer-service-info" ]; then
    message=`./$i --version -s httpg://foo 2>&1`
  else
    message=`./$i -s httpg://foo 2>&1`
  fi
  echo $message | grep 'could not open connection to'
  if [ $? -ne 0 ]; then
    echo "BUG PRESENT"
    exit 1
  else
    echo "OK"
  fi
done

for i in glite-transfer*;
do
  echo "$i -s http://foo"; 
  if [ "$i" == "glite-transfer-service-info" ]; then
    message=`./$i --version -s http://foo 2>&1`
  else
    message=`./$i -s http://foo 2>&1`
  fi
  echo $message | grep 'could not open connection to'
  if [ $? -ne 0 ]; then
    echo $message | grep 'Name or service not known'
    if [ $? -ne 0 ]; then
      echo "BUG PRESENT"
      exit 1
    fi
  else
    echo "OK"
  fi
done 

echo "BUG FIXED"
exit 0
