#!/bin/sh

#
# bug #35997: FTS cli: if an invalid value is passed with -s option, the default is used
#

export PATH="$PATH:/opt/glite/bin"

pushd /opt/glite/bin

for i in glite-transfer*; 
do 
  echo "\t$i -s httpj://foo"
  message=`./$i -s httpj://foo 2>&1`
  echo $message | grep 'Failed to determine the endpoint: Wrongly formatted endpoint'
  if [ $? -ne 0 ]; then
    echo "BUG PRESENT"
    exit 1
  fi
done

for i in glite-transfer*;
do 
  echo "\t$i  -s https://foo"; 
  message=`./$i -s https://foo 2>&1`
  echo $message | grep 'could not open connection to'
  if [ $? -ne 0 ]; then
    echo "BUG PRESENT"
    exit 1
  fi
done

for i in glite-transfer*; 
do 
  echo "\t$i  -s httpg://foo"; 
  message=`./$i -s httpg://foo 2>&1`   
  echo $message | grep 'could not open connection to'
  if [ $? -ne 0 ]; then
    echo "BUG PRESENT"
    exit 1
  fi
done

for i in glite-transfer*;
do
  echo "\t$i -s http://foo"; 
  message=`./$i -s http://foo 2>&1`   
  echo $message | grep 'could not open connection to'
  if [ $? -ne 0 ]; then
    echo "BUG PRESENT"
    exit 1
  fi
done 

echo "BUG FIXED"
exit 0
