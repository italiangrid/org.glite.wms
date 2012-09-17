#!/bin/bash

id=$1
echo "$id"

echo "$id" > "outdata_$id"

cat inpdata_$id
echo 
exit 0