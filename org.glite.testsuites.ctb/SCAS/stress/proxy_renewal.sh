#!/bin/sh

#This script creates a proxy certificate for test users 50x and
#renew the proxy every hour.
#The index that must be given as argument is used to choose 
#the user.
#The proxy is created into the current directory and copied
#into the pilot user account: pilotdteamx

function usage() {
echo "Usage: $0 <index>"
echo "  <index> integer number to discriminate clients in the test"
}

if [ $# -ne 1 ]; then
  usage
  exit 1
else
  INDEX=$1
fi

#TODO remove hardcoded path
echo "Retrieving test user cert and key"
cp --reply=yes /afs/cern.ch/project/gd/yaim-server/BitFaceCA/user_certificates/test_user_50${INDEX}_cert.pem ./
cp --reply=yes /afs/cern.ch/project/gd/yaim-server/BitFaceCA/user_certificates/test_user_50${INDEX}_key.pem ./
chown root ./test_user_50${INDEX}_cert.pem
chown root ./test_user_50${INDEX}_key.pem

echo "Creating proxy file x509up_u501_$INDEX"
echo "test" | glite-voms-proxy-init -q --voms dteam -cert ./test_user_50${INDEX}_cert.pem -key test_user_50${INDEX}_key.pem -out ./x509up_u501_$INDEX -pwstdin 2>&1 >> /dev/null
#true

if [ $? -ne 0 ]; then
  echo "Error creating the proxy"
  exit 1
fi

cp -f --reply=yes ./x509up_u501_$INDEX /home/dteampilot$INDEX
echo "User proxy ./x509up_u501_$INDEX succesfully created and stored in pilot user account"

echo "Entering the loop"
while [ 1 ] 
do
  sleep 1h
  echo "Creating proxy file x509up_u501_$INDEX"
  echo "test" | glite-voms-proxy-init -q --voms dteam -cert ./test_user_50${INDEX}_cert.pem -key test_user_50${INDEX}_key.pem -out ./x509up_u501_$INDEX -pwstdin 2>&1 >> /dev/null
  if [ $? -ne 0 ]; then
    echo "Error creating the proxy"
    exit 1
  fi
  echo "User proxy ./x509up_u501_$INDEX succesfully created"
done

