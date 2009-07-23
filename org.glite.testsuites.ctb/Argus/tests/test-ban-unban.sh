#!/bin/sh

PAP_HOME=/opt/authz/pap

echo `date`
echo "---Test-BAN/UNBAN---"
echo "1) testing user ban"

/opt/authz/pap/bin/pap-admin ban subject "/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name"

if [ $? -eq 0 ]; then
  echo "OK"
  echo "2) testing user unban"
  /opt/authz/pap/bin/pap-admin un-ban subject "/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name"
  if [ $? -eq 0 ]; then
    echo "OK"
    echo "3) testing unbanning non existing subject"
    /opt/authz/pap/bin/pap-admin un-ban subject "/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name"
    if [ $? -ne 0 ]; then
      echo "OK"
      echo "---Test-BAN/UNBAND: TEST PASSED---"
      echo `date`
      exit 0
    else
      echo "FAILED"
      echo "---Test-BAN/UNBAND: TEST FAILED---"
      echo `date`
      exit 1
    fi
  else
    echo "FAILED"
    echo "---Test-BAN/UNBAND: TEST FAILED---"
    echo `date`
    exit 1
  fi
else
  echo "FAILED"
  echo "---Test-BAN/UNBAND: TEST FAILED---"
  echo `date`
  exit 1
fi



