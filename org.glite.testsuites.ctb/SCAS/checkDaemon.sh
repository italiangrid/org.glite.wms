#!/bin/sh

echo "Checking the SCAS daemon is listening on port 8443"

netstat -platu | grep scas | grep 8443 | grep LISTEN

if [ $? -ne 0 ]; then
  echo "TEST FAILED"
  exit 1
else
  echo "TEST PASSED"
  exit 0
fi


