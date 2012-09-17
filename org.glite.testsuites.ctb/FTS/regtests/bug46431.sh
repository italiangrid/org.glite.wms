#!/bin/sh

#
#bug #46431: FTS: RFE transfer-status should report 'not found'
#

export GLITE_LOCATION="/opt/glite"
host=`hostname`
dummyid="11111-111-111-111-dummy"

glite-transfer-status -s https://${host}:8443/glite-data-transfer-fts/services/FileTransfer ${dummyid} > out.txt 2>&1

if [ $? -ne 0 ]; then
  grep 'not found' out.txt
  if [ $? -ne 0 ]; then
    echo "Not the expected error message"
    exit 1
  else
    echo "BUG FIXED"
    exit 0
  fi
else
  echo "Error code should have been !=0"
  exit 1
fi

