#!/bin/sh

if [ -z $PAP_HOME ]
then
    if [ -d /usr/share/argus/pap ]
    then
        PAP_HOME=/usr/share/argus/pap
    else
        if [ -d /opt/argus/pap ]
        then
            PAP_HOME=/opt/argus/pap
        else
            echo "PAP_HOME not defined"
            echo "/usr/share/argus/pap not found."
            echo "/opt/argus/pap not found"
            echo "Sort this out..."
            exit 2;
        fi
    fi
fi

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

#Remove all policies defined for the default pap
${PAP_HOME}/bin/pap-admin rap
if [ $? -ne 0 ]; then
  echo "Error cleaning the default pap"
  echo "Failed command: ${PAP_HOME}/bin/pap-admin rap"
  exit 1
fi

echo `date`
##############################################################
echo "---Test-BAN/UNBAN---"
echo "1) testing user ban"

${PAP_HOME}/bin/pap-admin ban subject "/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name"

if [ $? -eq 0 ]; then
  echo "OK"
  echo "2) testing user unban"
  ${PAP_HOME}/bin/pap-admin un-ban subject "/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name"
  if [ $? -eq 0 ]; then
    echo "OK"
    echo "3) testing unbanning non existing subject"
    ${PAP_HOME}/bin/pap-admin un-ban subject "/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name"
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



