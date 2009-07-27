#!/bin/sh

PAP_HOME=/opt/authz/pap

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

echo `date`
echo "---Test-BAN/UNBAN-FQAN---"
echo "1) testing fqan ban"

/opt/authz/pap/bin/pap-admin ban fqan "/badvo"

if [ $? -eq 0 ]; then
  echo "OK"
  echo "2) testing fqan unban"
  /opt/authz/pap/bin/pap-admin un-ban fqan "/badvo"
  if [ $? -eq 0 ]; then
    echo "OK"
    echo "3) testing unbanning non existing fqan"
    /opt/authz/pap/bin/pap-admin un-ban fqan "/badvo"
    if [ $? -ne 0 ]; then
      echo "OK"
      echo "---Test-BAN/UNBAND-FQAN: TEST PASSED---"
      echo `date`
      exit 0
    else
      echo "FAILED"
      echo "---Test-BAN/UNBAND-FQAN: TEST FAILED---"
      echo `date`
      exit 1
    fi
  else
    echo "FAILED"
    echo "---Test-BAN/UNBAND-FQAN: TEST FAILED---"
    echo `date`
    exit 1
  fi
else
  echo "FAILED"
  echo "---Test-BAN/UNBAND-FQAN: TEST FAILED---"
  echo `date`
  exit 1
fi



