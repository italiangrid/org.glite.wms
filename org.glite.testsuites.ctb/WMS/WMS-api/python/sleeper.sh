#!/bin/bash

MYFILE="out.txt"

echo "This is sleeper"
echo "This is sleeper" > out.txt

for ((i=1; i <= 500; i++))
do
  echo "I.time = $i`date +%T`" >> out.txt
  sleep 1

done

echo "Stop sleeping!" >> out.txt
echo "Stop sleeping!"
