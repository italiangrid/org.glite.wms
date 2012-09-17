#!/bin/bash
showUsage ()
{
cat <<EOF

Usage: myproxy-testi2.sh [--options] MyProxy_HOST
   1. Store a credential (myproxy-init).
   2. Retrieve stored credential (myproxy-logon).
   3. Verify myproxy-logon fails on bad passphrase.
   4. Change passphrase (myproxy-change-pass-phrase).
   5. Retrieve with new passphrase  (myproxy-logon).
   6. Verify proxy (grid-proxy-info -file /tmp/myproxy-test.$$)
   7. Verify old passphrase no longer accepted.
   8. Verify renewal fails by default.
   9. Remove credential from repository (myproxy-destroy).
  Options :
    -help                           displays usage
    -h                              displays usage
    --help                          displays usage
EOF
}
if [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$1" = "--help" ]; then
  showUsage
  exit 1
fi
. setup_env
. myproxy-functions.sh
MYPROXY_SERVER=$1
if [ "$MYPROXY_SERVER" == "" ]
then 
  myexit 1 "MYPROXY_SERVER is not defined"
fi 
ERR=0
report=""
vomsproxyinfo
get_cert_subject
passphrase=`date +%s`
passphrase=$RANDOM
let passphrase=passphrase+1000001
echo $passphrase | myproxy-init -v -a -c 1 -t 1 -S 2>&1 
if [ "$?" -ne 0 ]
then
   myexit 1 "myproxy-init -v -a -c 1 -t 1 -S"
fi  
  myecho "myproxy-init is succeeded"
##############################################
#  Retrieve stored credential (myproxy-logon).
##############################################
echo $passphrase | myproxy-logon -t 1 -o /tmp/myproxy-test.$$ -v -S 2>&1 
if [ "$?" -ne 0 ]
then 
 myexit 1 "myproxy-logon -t 1 -o /tmp/myproxy-test.$$ -v -S "
fi
   myecho "myproxy-logon  is succeeded"
#grid-proxy-info -file /tmp/myproxy-test.$$
###############################################
# Verify myproxy-logon fails on bad passphrase.
###############################################
run_command verifyproxy /tmp/myproxy-test.$$
old_passphrase=$passphrase
passphrase=`date +%s`
passphrase=$RANDOM
let passphrase=passphrase+1000001
################################################
# Change passphrase (myproxy-change-pass-phrase)
################################################
( echo $old_passphrase ; echo $passphrase ) | myproxy-change-pass-phrase -v -S 2>&1
if [ "$?" -ne 0 ]
then
   ERR=1
   report="myproxy-change-pass-phrase"
   myecho "myproxy-change-pass-phrase is failed"
fi
   myecho "myproxy-change-pass-phrase is succeeded"
 echo $passphrase | myproxy-logon -t 1 -N -v -S 2>&1
if [ "$?" -ne 0 ]
then 
 myexit 1 "myproxy-logon with new passphrase"
fi
   myecho "myproxy-logon is succeeded with new passphrase"   
   
#################################   
#  Retrieve with new passphrase.
#################################
run_command verifyproxy /tmp/myproxy-test.$$
##########################################
#Verify old passphrase no longer accepted.
##########################################
echo $old_passphrase | myproxy-logon -t 1 -o /tmp/myproxy-test.$$ -v -S 2>&1
 if [ "$?" = "0" ]
then 
 myecho "verify old passphrase fails is error"
 ERR=1
 report=`echo $report invalid pass phrase is error`
  report=`echo $report default renewal policy`
fi
   myecho "verify old passphrase fails succeeded"
#################################
#Verify renewal fails by default.
#################################
 myproxy-logon -a $X509_USER_PROXY -t 1 -o /tmp/myproxy-test.$$ -v 2>&1
 if [ "$?" = "0" ]
then 
 myecho "verify default renewal policy"
 ERR=1
 report=`echo $report default renewal policy`
fi
   myecho "verify old passphrase fails succeeded"    
run_command myproxy-destroy -v -s $MYPROXY_SERVER
rm -f /tmp/myproxy-test* 2>&1
rm -f /tmp/tmpproxy*
if [ "$ERR" -ne "0" ]; then
myexit 1 "$report"
fi 
myexit 0
