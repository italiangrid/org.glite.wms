#!/bin/bash

tout=$1  #timeout in sec
let ncyc=$2  #the number of cycles
#let ncyc=2

sleep "$tout" 2>&1 > tmp1 && kill -s 14 $$ 2>&1 > tmp1  &  #timeout signal

trap timeout 14

timeout(){
 echo "Timeout" >> out.txt
 exit 0
}

echo "Start: echo Welcome" > out.txt
echo "Welcome!"
echo "Step 0: Waiting for the reply to Welcome" >> out.txt
read data
echo "0: Get data \"$data\" (Length=${#data})" >> out.txt
             #main dialog cycle
let i=1
while [ $i -le $ncyc ]; do
 echo "Step $i: Waiting data" >> out.txt
 data=''
 while [ -z $data ]; do
  read data
  len=${#data}
  sleep 2
 done
 echo "$i: Get data \"$data\" (Length=$len)" >> out.txt
 echo "$i: Get data \"$data\" (Length=$len)"
 let i=i+1
 sleep 5
done

kill -9 $! &>/dev/null 
echo "Finish:" >> out.txt
exit 0

