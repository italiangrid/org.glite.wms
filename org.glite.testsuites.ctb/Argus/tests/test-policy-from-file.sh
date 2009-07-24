#!/bin/sh

PAP_HOME=/opt/authz/pap
policyfile=policyfile.txt
failed="no"

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

echo `date`
echo "---Test-APF---"
#########################################################
echo "1) testing add policy from file"
cat <<EOF > $policyfile
resource ".*" {

    action ".*" {
        rule deny { subject="/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name" }
    }
}
resource ".*" {

    action ".*" {
        rule deny { fqan="/badvo" }
    }
}
EOF

/opt/authz/pap/bin/pap-admin apf $policyfile
if [ $? -eq 0 ]; then
  echo "OK"
else
  echo "FAILED"
  failed="yes"
fi

/opt/authz/pap/bin/pap-admin un-ban subject "/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name"
/opt/authz/pap/bin/pap-admin un-ban fqan "/badvo"

#########################################################
echo "2) testing add policy from file with error"
cat <<EOF > $policyfile
resource ".*" {

    action ".*" {
        rule deni { subject="/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name" }
    }
}
EOF

/opt/authz/pap/bin/pap-admin apf $policyfile
if [ $? -ne 0 ]; then
  echo "OK"
else
  echo "FAILED"
  failed="yes"
fi

rm -f $policyfile

if [ $failed == "yes" ]; then
  echo "---Test-APF: TEST PASSED---"
  echo `date`
  exit 0
else
  echo "---Test-APF: TEST PASSED---"
  echo `date`
  exit 1
fi

