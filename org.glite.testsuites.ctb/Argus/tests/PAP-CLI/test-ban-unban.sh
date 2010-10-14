#!/bin/sh

PAP_HOME=/opt/argus/pap

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

#Remove all policies defined for the default pap
/opt/argus/pap/bin/pap-admin rap
if [ $? -ne 0 ]; then
  echo "Error cleaning the default pap"
  echo "Failed command: /opt/argus/pap/bin/pap-admin rap"
  exit 1
fi

echo `date`
##############################################################
echo "---Test-BAN/UNBAN---"
echo "1) testing user ban"

/opt/argus/pap/bin/pap-admin ban subject "/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name"

if [ $? -eq 0 ]; then
  echo "OK"
  echo "2) testing user unban"
  /opt/argus/pap/bin/pap-admin un-ban subject "/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name"
  if [ $? -eq 0 ]; then
    echo "OK"
    echo "3) testing unbanning non existing subject"
    /opt/argus/pap/bin/pap-admin un-ban subject "/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name"
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



