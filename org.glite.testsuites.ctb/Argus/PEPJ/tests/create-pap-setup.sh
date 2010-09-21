#!/bin/bash

PROXY=${X509_USER_PROXY:-/tmp/x509_up`id -u`}
CERT=$PROXY
KEY=$PROXY
VO=dteam


usage() {
 echo
 echo "Usage:"
 echo "======"
 echo "create-pap-setup.sh "
 echo "Creates setup files for configuring the Argus PAP under 'pap-config'"
 echo "Once created, please copy the directoryit over to the machine with the pap"
 echo "and run the 'configure-pap.sh from under it."
 echo ""
}

while [ $# -gt 0 ]
do
 case $1 in
 --help | -help | --h | -h ) usage
  exit 0
  ;;
 --* | -* ) echo "$0: invalid option $1" >&2
  usage
  exit 1
  ;;
 *) break
  ;;
 esac
 shift
done

grid-cert-info >/dev/null 2>&1 

if [ $? -ne 0 ] ; then
 echo "Error, could not find a certificate to use. Please run this as the user who will run the"
 echo "certification tests, and make sure there's a certificate installed for the user."
 exit 1
fi


IFS=/ SUBJ=`grid-cert-info -subject`

i=0
for part in $SUBJ ; do 
 NEWSUBJ[i]=$part 
 let i+=1
done

FINALSUBJ=""
while [ $i -ge 0 ] ; do 
  if [ x$FINALSUBJ == "x" ] ; then 
   FINALSUBJ=${NEWSUBJ[$i]}
  else 
   if [ x${NEWSUBJ[$i]} != "x" ] ; then 
    FINALSUBJ=$FINALSUBJ,${NEWSUBJ[$i]}
   fi
  fi
  let i-=1
done

echo "Using $FINALSUBJ as certificate subject"
echo "Using $VO as the user's VO"

sed  "s/__VO__/$VO/" ./pap-config/policy1-templ > ./pap-config/policy1
sed  "s/__SUBJECT__/$FINALSUBJ/" ./pap-config/policy2-templ > ./pap-config/policy2
sed  "s/__SUBJECT__/$FINALSUBJ/" ./pap-config/policy3-templ > ./pap-config/policy3

echo "Configuration creation done. Please copy over the 'pap-config' direcotry to the pap node,"
echo "and run pap-config/configure-pap.sh there"
