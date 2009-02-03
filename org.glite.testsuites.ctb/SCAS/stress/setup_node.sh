#!/bin/sh


#TODO
#Redirect all the output messages to the log file

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
#rm -f test_user_*.pem x509up_u501*
#cp /afs/cern.ch/project/gd/yaim-server/BitFaceCA/user_certificates/test_user_50${INDEX}_cert.pem ./
#cp /afs/cern.ch/project/gd/yaim-server/BitFaceCA/user_certificates/test_user_50${INDEX}_key.pem ./

#echo "test" | glite-voms-proxy-init -quiet --voms dteam -cert ./test_user_50${INDEX}_cert.pem -key test_user_50${INDEX}_key.pem -out ./x509up_u501_$INDEX -pwstdin 2>&1 >> /dev/null
#true

#if [ $? -ne 0 ]; then
#  echo "Error creating the proxy"
#  exit 1
#fi
#echo "User proxy ./x509up_u501_$INDEX succesfully created"

#Retrieve the configuration file
source /afs/cern.ch/user/p/pucciani/public/glitetests/src/org.glite.testsuites.ctb/SCAS/stress/setup_test.cfg
if [ $? -ne 0 ]; then
  echo "Error sourcing the setup_test.cfg file"
  exit 1
fi


echo "Retrieving glexec test script"
cp -f --reply=yes $glexec_stress_script ./
echo "Retrieving proxy renewal script"
cp -f --reply=yes $proxy_ren_script ./
echo "Starting proxy renewal daemon"
./proxy_renewal.sh $INDEX &
proxy_renewal_pid=$! 

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

cp -f ./glexec_stress_test.sh /home/dteamdteampilot$INDEX >> $LOG_FILE 2>&1
chown dteamdteampilot$INDEX /home/dteamdteampilot$INDEX/glexec_stress_test.sh >> $LOG_FILE 2>&1
chown dteamdteampilot$INDEX /home/dteamdteampilot$INDEX/x509up_u501_$INDEX >> $LOG_FILE 2>&1
chmod u+x /home/dteamdteampilot$INDEX/glexec_stress_test.sh >> $LOG_FILE 2>&1


echo "Starting glexec test as dteamdteampilot$INDEX"
#su - -c "/home/dteamdteampilot$INDEX/glexec_stress_test.sh -f $LOG_FILE -n 3 -i $INDEX" dteamdteampilot$INDEX
su - -c "/home/dteamdteampilot$INDEX/glexec_stress_test.sh -f $LOG_FILE -d 200902041400 -i $INDEX" dteamdteampilot$INDEX

#kill the proxy renewal process
kill $proxy_renewal_pid
if [ $? -eq 0 ];then
  echo "Proxy renewal daemon killed"
else
  echo "Error in killing proxy renewal daemon, pid is $proxy_renewal_pid"
fi

exit 0

