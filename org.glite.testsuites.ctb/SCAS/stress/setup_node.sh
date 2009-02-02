#!/bin/sh

function usage() {
echo "Usage: ./setup_node.sh <index> <log file>"
echo "    <index> integer number to identify the node."
echo "    <log file> the file that will be used to log data and start the test."
}

if [ $# -ne 2 ]; then
  usage
  exit 1
fi

INDEX=$1

if [ ! -f $2 ]; then
  echo "The second argument must be a valid log file"
  usage
  exit 1
else
  LOG_FILE=$2
fi


#get test_user_50x cert/key
rm -f test_user_*.pem x509up_u501*
cp /afs/cern.ch/project/gd/yaim-server/BitFaceCA/user_certificates/test_user_50${INDEX}_cert.pem ./
cp /afs/cern.ch/project/gd/yaim-server/BitFaceCA/user_certificates/test_user_50${INDEX}_key.pem ./

#echo "test" | glite-voms-proxy-init -quiet --voms dteam -cert ./test_user_50${INDEX}_cert.pem -key test_user_50${INDEX}_key.pem -out ./x509up_u501_$INDEX -pwstdin 2>&1 >> /dev/null
#true

#if [ $? -ne 0 ]; then
#  echo "Error creating the proxy"
#  exit 1
#fi
#echo "User proxy ./x509up_u501_$INDEX succesfully created"


echo "Retrieving glexec test script"
cp -f --reply=yes /afs/cern.ch/user/p/pucciani/public/glitetests/src/org.glite.testsuites.ctb/SCAS/stress/glexec_stress_test.sh ./
echo "Retrieving proxy renewal script"
cp -f --reply=yes /afs/cern.ch/user/p/pucciani/public/glitetests/src/org.glite.testsuites.ctb/SCAS/stress/proxy_renewal.sh ./
echo "Starting proxy renewal daemon"
./proxy_renewal.sh $INDEX &

#Give time to create the proxy
sleep 10s

#CHANGE THIS: use special certificate
#cp -f /afs/cern.ch/user/p/pucciani/tmp/x509up_u501_g /home/dteampilot$INDEX/x509up_u501_$INDEX 2>&1 >> $LOG_FILE

#echo "Copying test and proxy to ~dteampilot$INDEX"

#cp -f x509up_u501_$INDEX /home/dteampilot$INDEX 2>&1 >> $LOG_FILE
#if [ $? -ne 0 ]; then
#  echo "Error copying the proxy"
#  exit 1
#fi

cp -f ./glexec_stress_test.sh /home/dteampilot$INDEX 2>&1 >> $LOG_FILE
chown dteampilot$INDEX /home/dteampilot$INDEX/glexec_stress_test.sh 2>&1 >> $LOG_FILE
chown dteampilot$INDEX /home/dteampilot$INDEX/x509up_u501_$INDEX 2>&1 >> $LOG_FILE
chmod u+x /home/dteampilot$INDEX/glexec_stress_test.sh 2>&1 >> $LOG_FILE


echo "Starting glexec test as dteampilot$INDEX"
su - -c "/home/dteampilot$INDEX/glexec_stress_test.sh -f $LOG_FILE -n 2 -i $INDEX" dteampilot$INDEX
#su - -c "/home/dteampilot$INDEX/glexec_stress_test.sh -f $LOG_FILE -d 200902010830 -i $INDEX" dteampilot$INDEX

exit 0

