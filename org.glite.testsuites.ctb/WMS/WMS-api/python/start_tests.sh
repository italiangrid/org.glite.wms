#!/bin/bash
#The WMSLB_HOST name is obligatory commandline argument
#example: ./start_tests.sh lxb2054.cern.ch
#example: ./start_tests.sh lxb2054.cern.ch -output std.out
#
if test -z "$1"
then
   echo "The WMSLB_HOST name is not defined"
   echo "usage: ./start_tests.sh WMSLB_HOST [-output outfile]"
   exit 1
fi
nodeName=$1
Err=0
logoutput=""
echo $nodeName
if [ "$2" == "-output" ]
then
logoutput=$3
      if   test -e "$logoutput"
      then
      rm -f $logoutput
      fi
fi
for test in `cat test-sequence.lst` 
 do
       if test "$3" = ""

       then
         tests/$test $nodeName
       else
         tests/$test $nodeName >> $logoutput
       fi
       if [[ $? ==  0 ]] ; then
              echo  "Test $test OK"
       else 
              echo "Test $test has ERROR"
              Err=1 
       fi
 done 
exit $Err
