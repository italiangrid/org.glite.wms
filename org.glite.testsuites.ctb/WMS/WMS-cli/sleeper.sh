#!/bin/sh

MYFILE="out.txt"

echo "This is sleeper"
echo "This is sleeper" > out.txt

for ((i=1; i <= 50; i++))
do

  echo "message $i" >> out.txt
  sleep 1

done

echo "Stop sleeping!" >> out.txt
echo "Stop sleeping!"
