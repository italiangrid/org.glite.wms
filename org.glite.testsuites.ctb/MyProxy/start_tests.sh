#!/bin/bash
#Author Liudmila Stepanova < sli@inr.ru >
#example: ./start_tests.sh [MYPROXY_SERVER]
#
number="$#"
if [ "$number" -eq 1 ]
then
    nodeName=$1
    export MYPROXY_SERVER=$nodeName
fi
echo "The MYPROXY_HOST  is $MYPROXY_SERVER"
if [ "$MYPROXY_SERVER" == "" ]
then
  echo MYPROXY_SERVER is not defined
  exit 1
fi
export X509_USER_PROXY=`voms-proxy-info -path`
echo X509_USER_PROXY=$X509_USER_PROXY
Err=0
for test in `cat test-sequence.lst` 
 do
         tests/$test $MYPROXY_SERVER
       if [ $? ==  0 ] ; then
              echo  "Test $test OK"
       else 
              echo "Test $test has ERROR"
              Err=1 
       fi
 done 
exit $Err
