#!/bin/sh

#
# bug #33641 and #60095: In some cases, the delegated proxies got corrupted during FTS run
#
# This bug requires sqlplus to be installed
#
# This test must be executed from an UI
#
# test-glite-delegation-bug-33641 is a binary file. If you have problems with it
# you may recompile it in ETICS (org.glite/data/delegation-cli)

# Some configuration parameters needed to test this bug
# Note: this can be obtained from the FTS site-info.def file
FTS_DB_USER=YOUR_USER
FTS_DB_PASSWORD=YOUR_PASSWORD
FTS_DB_CONNECTION_STRING="(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(HOST=int11r2-v.cern.ch)(PORT=10121))(ADDRESS=(PROTOCOL=TCP)(HOST=int11r1-v.cern.ch)(PORT=10121))(LOAD_BALANCE=yes)(CONNECT_DATA=(SERVER=DEDICATED)(SERVICE_NAME=lcg_fts_int11r.cern.ch)))"

FTS_HOST=vtb-generic-85

# Check if sqlplus is installed
which sqlplus &> /dev/null

if [ $? -ne 0 ]; then
  echo "SQLPlus is needed in this node in order to test this bug"
  echo ">> yum -y --enablerepo CERN-only install oracle-instantclient-basic oracle-instantclient-sqlplus"
  exit 1
fi

# Test proxy
voms-proxy-info &> /dev/null
if [ $? -ne 0 ]; then
  echo "There isn't a valid proxy, please, use voms-proxy-init"
  echo "Make sure the proxy is long enough!!"
  exit 1
fi

# Endpoint
WEB_SERVICE_ENDPOINT=https://$FTS_HOST:8443/glite-data-transfer-fts/services/gridsite-delegation

echo "Using endpoint"
echo $WEB_SERVICE_ENDPOINT

# Destroy and recreate the delegation
echo "Recreating proxy delegation"
glite-delegation-destroy -s $WEB_SERVICE_ENDPOINT &> /dev/null
glite-delegation-init -s $WEB_SERVICE_ENDPOINT &> /dev/null

if [ $? -ne 0 ]; then
  echo "Error while trying to get the proxy delegation. Maybe your certificate expires too soon. Try"
  echo "voms-proxy-init --voms dteam --hour 240"
  exit 1
fi

# Obtains the delegation ID
DELEGATION_ID=`glite-delegation-init -s $WEB_SERVICE_ENDPOINT 2>&1 | awk -F "'" '{print $2}'`
echo "The delegation ID is $DELEGATION_ID"

# Test the bug
echo "Executing the test case"
glite-delegation-destroy -s $WEB_SERVICE_ENDPOINT &> /dev/null
./test-glite-delegation-bug-33641 -f -s $WEB_SERVICE_ENDPOINT &> /dev/null

# Extract the proxy from SQLPlus
echo "Extracting the proxy from the database"
PROXY_FILE="/tmp/proxy.$$"

SQL_COMMAND="set long 10000; set pagesize 999; select proxy from t_credential where dlg_id='$DELEGATION_ID';"

CONNECTION_STRING=$FTS_DB_USER/$FTS_DB_PASSWORD@$FTS_DB_CONNECTION_STRING

sqlplus -S $CONNECTION_STRING << EOF > $PROXY_FILE
set pagesize 999;
set long 10000;
select proxy from t_credential where dlg_id='$DELEGATION_ID';
EOF


# Clean proxy file?

# Extract x509 modulus
X509_MODULUS=`openssl x509 -in $PROXY_FILE -noout -modulus`
if [ $? -ne 0 ]; then
  echo "TEST FAILED: Error while extracting x509 modulus"
  exit 1
fi

# Extract rsa modulus
RSA_MODULUS=`openssl rsa -in $PROXY_FILE -noout -modulus`
if [ $? -ne 0 ]; then
  echo "TEST FAILED: Error while extracting RSA modulus"
  exit 1
fi

# Show log
echo
echo

echo "x509 modulus:"
echo "============="
echo $X509_MODULUS
echo
echo "RSA modulus:"
echo "============="
echo $RSA_MODULUS

echo
echo

if [ $X509_MODULUS == $RSA_MODULUS ]; then
  echo "TEST OK"
  RESULT=0
else
  echo "TEST FAILED: The proxy is corrupted"
  RESULT=1
fi

# Clean temporary files
rm -f $PROXY_FILE &> /dev/null

# End
exit $RESULT

