#!/bin/bash
#The WMS_HOST name is obligatory commandline argument
#example: ./start_tests.sh -r grmpice.inr.troitsk.ru -o out grinr05.inr.troitsk.ru
#
if test -z "$1"
then
   echo "The WMSLB_HOST name is not defined"
   echo "./start_tests.sh [options]  < WmsProxy_id >"
   echo "options:"
   echo "        -r    <ce_id>"
   echo "        -o    <file_path>"
   exit 1
fi
number="$#"
if [ "$number" -eq 1 ]
then 
    nodeName=$1
    echo $nodeName
else
  logoutput=""
  req=""
  i=1
   
  while [ "$i" -le "$number" ] 
       do
         if test "$1" = "-r"
         then
	    let  i=$i+1
	    shift
            req="$1"; shift
         else
           if test "$1" = "-o"
             then
	      let  i=$i+1 
              shift; logoutput="$1."`date +%s` ; shift
           fi
          fi
	  let  i=$i+1
       done
 
    nodeName=$1
fi
Err=0
echo "nodeName=" $nodeName
echo "OUT=" $logoutput
echo "REQ=" $req

for test in `cat test-sequence.lst` 
 do
       if test "$logoutput" = ""

       then
         tests/$test  $nodeName $req
       else
         tests/$test $nodeName $req >> $logoutput
       fi
       if [ $? ==  0 ] ; then
              echo  "Test $test OK"
       else 
              echo "Test $test has ERROR"
              Err=1 
       fi
 done 
exit $Err
