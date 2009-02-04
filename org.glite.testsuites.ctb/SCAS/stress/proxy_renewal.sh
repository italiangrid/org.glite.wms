#!/bin/sh

#This script creates a proxy certificate for test users 50x and
#renew the proxy every hour.
#The index that must be given as argument is used to choose 
#the user.
#The proxy is created into the current directory and copied
#into the pilot user account: dteamdteampilotx
#The output is stored in proxy_renewal.out

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

#Create log file
logfile=$0.out
rm -f $logfile
touch $logfile


#TODO remove hardcoded path
echo "Retrieving test user cert and key"
cp --reply=yes $users_certs_dir/test_user_50${INDEX}_cert.pem ./
cp --reply=yes $users_certs_dir/test_user_50${INDEX}_key.pem ./
chown root ./test_user_50${INDEX}_cert.pem
chown root ./test_user_50${INDEX}_key.pem

export PATH="$PATH:/opt/glite/bin/"

echo "Creating proxy file x509up_u501_$INDEX" >> $logfile 
echo "test" | glite-voms-proxy-init -q --voms dteam -cert ./test_user_50${INDEX}_cert.pem -key test_user_50${INDEX}_key.pem -out ./x509up_u501_$INDEX -pwstdin >> $logfile 2>&1
#true

if [ $? -ne 0 ]; then
  echo "Error creating the proxy" >> $logfile
  exit 1
fi

cp -f --reply=yes ./x509up_u501_$INDEX /home/dteamdteampilot$INDEX
echo "User proxy ./x509up_u501_$INDEX succesfully created and stored in pilot user account" >> $logfile

echo "Entering the loop" >> $logfile
while [ 1 ] 
do
  sleep 1h
  echo "Creating proxy file x509up_u501_$INDEX" >> $logfile
  echo "test" | glite-voms-proxy-init -q --voms dteam -cert ./test_user_50${INDEX}_cert.pem -key test_user_50${INDEX}_key.pem -out ./x509up_u501_$INDEX -pwstdin >> $logfile 2>&1
  if [ $? -ne 0 ]; then
    echo "Error creating the proxy" >> $logfile
    exit 1
  fi
  echo "User proxy ./x509up_u501_$INDEX succesfully created" >> $logfile
done

