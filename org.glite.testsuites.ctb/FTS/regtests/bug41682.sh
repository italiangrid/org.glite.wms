#!/bin/sh

export PATH="$PATH:/opt/glite/bin"
glite-transfer-submit --help | grep '\-\-destination' >>/dev/null

if [ $? -ne 0 ]; then
  echo "BUG PRESENT"
  exit 1
else
  echo "BUG FIXED"
  exit 0
fi
