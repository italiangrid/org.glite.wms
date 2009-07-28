#!/bin/bash

#Assumtpions: The PAP is running with a correct configuration file
#Note: Each single test has this assumption

echo `date`
echo "---Test-authz---"

authzconffile=$PAP_HOME/conf/pap_authorization.ini
authzbkpfile=$PAP_HOME/conf/pap_authorization.bkp
failed="no"

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

#################################################################
echo "1) testing lp with no authorization"
mv -f $authzconffile  $authzbkpfile

cat <<EOF > $authzconffile
# Configuration file created by YAIM
[dn]
ANYONE : CONFIGURATION_READ
[fqan]
EOF

/etc/rc.d/init.d/pap-standalone restart >>/dev/null
sleep 10
/opt/authz/pap/bin/pap-admin lp >>/dev/null
if [ $? -eq 0 ]; then
  failed="yes"
  echo "FAILED"
else
  echo "OK"
fi

#################################################################
#start/restart the server
mv -f $authzbkpfile $authzconffile
/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP not running'
if [ $? -eq 0 ]; then
  /etc/rc.d/init.d/pap-standalone start >>/dev/null
else
  /etc/rc.d/init.d/pap-standalone restart >>/dev/null
fi
sleep 10

#################################################################
echo "1) testing lp with anyone full power"
mv -f $authzconffile  $authzbkpfile

cat <<EOF > $authzconffile
# Configuration file created by YAIM
[dn]
ANYONE : ALL
[fqan]
EOF

/etc/rc.d/init.d/pap-standalone restart >>/dev/null
sleep 10
/opt/authz/pap/bin/pap-admin lp >>/dev/null
if [ $? -ne 0 ]; then
  failed="yes"
  echo "FAILED"
else
  echo "OK"
fi

#################################################################
#start/restart the server
mv -f $authzbkpfile $authzconffile
/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP not running'
if [ $? -eq 0 ]; then
  /etc/rc.d/init.d/pap-standalone start >>/dev/null
else
  /etc/rc.d/init.d/pap-standalone restart >>/dev/null
fi
sleep 10

if [ $failed == "yes" ]; then
  echo "---Test-authz: TEST FAILED---"
  echo `date`
  exit 1
else
  echo "---Test-authz: TEST PASSED---"
  echo `date`
  exit 0
fi

