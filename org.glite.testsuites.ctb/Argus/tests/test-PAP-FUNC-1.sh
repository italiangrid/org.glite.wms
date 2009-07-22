#!/bin/bash

#Assumtpions: The PAP is running with a correct configuration file

echo `date`
echo "---Test-PAP-FUNC-1---"

conffile=$PAP_HOME/conf/pap_configuration.ini
bkpfile=$PAP_HOME/conf/pap_configuration.ini.bkp
failed="no"

echo "1) testing required security section"
mv -f $conffile  $bkpfile
cat <<EOF > $conffile
[paps]
## Trusted PAPs will be listed here

[paps:properties]

poll_interval = 14400
ordering = default

[repository]

location = /opt/authz/pap/repository
consistency_check = false
consistency_check.repair = false

[standalone-service]

hostname = 127.0.0.1
port = 8150
shutdown_port = 8151

[security]

#certificate = /etc/grid-security/hostcert.pem
#private_key = /etc/grid-security/hostkey.pem

EOF

/etc/rc.d/init.d/pap-standalone restart >>/dev/null
sleep 10
/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  mv -f $bkpfile $conffile
  echo "FAILED"
else
  mv -f $bkpfile $conffile
  /etc/rc.d/init.d/pap-standalone start >>/dev/null
  sleep 10
  echo "OK"
fi

echo "2) testing required poll_interval "
mv -f $conffile  $bkpfile
cat <<EOF > $conffile
[paps]
## Trusted PAPs will be listed here

[paps:properties]

#poll_interval = 14400
ordering = default

[repository]

location = /opt/authz/pap/repository
consistency_check = false
consistency_check.repair = false

[standalone-service]

hostname = 127.0.0.1
port = 8150
shutdown_port = 8151

[security]

certificate = /etc/grid-security/hostcert.pem
private_key = /etc/grid-security/hostkey.pem

EOF

/etc/rc.d/init.d/pap-standalone restart >>/dev/null
sleep 10
/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  echo "FAILED"
  mv -f $bkpfile $conffile
else
  mv -f $bkpfile $conffile
  /etc/rc.d/init.d/pap-standalone start >>/dev/null
  sleep 10
  echo "OK"
fi

echo "3) testing syntax error: missing ']'"
mv -f $conffile  $bkpfile
cat <<EOF > $conffile
[paps]
## Trusted PAPs will be listed here

[paps:properties]

poll_interval = 14400
ordering = default

[repository]

location = /opt/authz/pap/repository
consistency_check = false
consistency_check.repair = false

[standalone-service

hostname = 127.0.0.1
port = 8150
shutdown_port = 8151

[security]

certificate = /etc/grid-security/hostcert.pem
private_key = /etc/grid-security/hostkey.pem

EOF

/etc/rc.d/init.d/pap-standalone restart >>/dev/null
sleep 10
/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  echo "FAILED"
  mv -f $bkpfile $conffile
else
  mv -f $bkpfile $conffile
  /etc/rc.d/init.d/pap-standalone start >>/dev/null
  sleep 10
  echo "OK"
fi

echo "4) testing syntax error: missing '='"
mv -f $conffile  $bkpfile
cat <<EOF > $conffile
[paps]
## Trusted PAPs will be listed here

[paps:properties]

poll_interval = 14400
ordering  default

[repository]

location = /opt/authz/pap/repository
consistency_check = false
consistency_check.repair = false

[standalone-service]

hostname = 127.0.0.1
port = 8150
shutdown_port = 8151

[security]

certificate = /etc/grid-security/hostcert.pem
private_key = /etc/grid-security/hostkey.pem

EOF

/etc/rc.d/init.d/pap-standalone restart >>/dev/null
sleep 10
/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  echo "FAILED"
  mv -f $bkpfile $conffile
else
  mv -f $bkpfile $conffile
  /etc/rc.d/init.d/pap-standalone start >>/dev/null
  sleep 10
  echo "OK"
fi

#Restore the bkp file and restart the server
/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP not running'
if [ $? -eq 0 ]; then
  /etc/rc.d/init.d/pap-standalone start >>/dev/null
else
  /etc/rc.d/init.d/pap-standalone restart >>/dev/null
fi
sleep 10

if [ $failed == "yes" ]; then
  echo "---Test-PAP-FUNC-1: TEST FAILED---"
  echo `date`
  exit 1
else
  echo "---Test-PAP-FUNC-1: TEST PASSED---"
  echo `date`
  exit 0
fi

