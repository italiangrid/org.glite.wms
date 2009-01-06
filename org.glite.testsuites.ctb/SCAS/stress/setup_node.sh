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


#get test_user_10$1 cert/key and create a proxy
rm -f test_user_*.pem x509up_u501*
cp /afs/cern.ch/project/gd/yaim-server/BitFaceCA/user_certificates/test_user_101_cert.pem ./
cp /afs/cern.ch/project/gd/yaim-server/BitFaceCA/user_certificates/test_user_101_key.pem ./

echo "test" | glite-voms-proxy-init -quiet --voms dteam -cert ./test_user_101_cert.pem -key test_user_101_key.pem -out ./x509up_u501_$INDEX -pwstdin 2>&1 >> /dev/null
true
if [ $? -ne 0 ]; then
  echo "Error creating the proxy"
  exit 1
fi
echo "User proxy ./x509up_u501_$INDEX succesfully created"

#start proxy renewal daemon
echo "Starting proxy renewal daemon"
#TODO!!

echo "Retrieving glexec test"
cp -f --reply=yes /afs/cern.ch/user/p/pucciani/public/glitetests/scas/stress/glexec_stress_test.sh ./

echo "Copying test and proxy to ~pildtm0$INDEX"

#CHANGE THIS
#cp -f /afs/cern.ch/user/p/pucciani/tmp/x509up_u501_2 /home/pildtm0$INDEX/x509up_u501_$INDEX 2>&1 >> $LOG_FILE
cp -f x509up_u501_$INDEX /home/pildtm0$INDEX 2>&1 >> $LOG_FILE
cp -f ./glexec_stress_test.sh /home/pildtm0$INDEX 2>&1 >> $LOG_FILE
chown pildtm0$INDEX /home/pildtm0$INDEX/glexec_stress_test.sh 2>&1 >> $LOG_FILE
chown pildtm0$INDEX /home/pildtm0$INDEX/x509up_u501_$INDEX 2>&1 >> $LOG_FILE
chmod u+x /home/pildtm0$INDEX/glexec_stress_test.sh 2>&1 >> $LOG_FILE


echo "Starting glexec test as pildtm0$INDEX"
su - -c "/home/pildtm0$INDEX/glexec_stress_test.sh -f $LOG_FILE -n 2 -i $INDEX" pildtm0$INDEX
#su - -c "/home/pildtm0$INDEX/glexec_stress_test.sh -f $LOG_FILE -d 200812121537 -i $INDEX" pildtm0$INDEX

exit 0

