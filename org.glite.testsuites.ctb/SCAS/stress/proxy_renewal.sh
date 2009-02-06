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


#TODO remove hardcoded path
echo "Retrieving test user cert and key"
cp --reply=yes $users_certs_dir/test_user_50${INDEX}_cert.pem ./
cp --reply=yes $users_certs_dir/test_user_50${INDEX}_key.pem ./
chown root ./test_user_50${INDEX}_cert.pem
chown root ./test_user_50${INDEX}_key.pem

export PATH="$PATH:/opt/glite/bin/"

echo "Creating proxy file x509up_u501_$INDEX"  
echo "test" | glite-voms-proxy-init -q --voms dteam -cert ./test_user_50${INDEX}_cert.pem -key test_user_50${INDEX}_key.pem -out ./x509up_u501_$INDEX -pwstdin 
#true
if [ $? -ne 0 ]; then
  echo "Error creating the proxy" 
  exit 1
fi
cp --reply=yes ./x509up_u501_$INDEX /home/dteamdteampilot$INDEX
if [ $? -ne 0 ]; then
  echo "Error copying the proxy" 
  exit 1
fi
echo "User proxy ./x509up_u501_$INDEX succesfully created and stored in pilot user account" 

echo "Entering the loop" 
while [ 1 ] 
do
  sleep 4h
  echo "Creating proxy file x509up_u501_$INDEX" 
  echo "test" | glite-voms-proxy-init -q --voms dteam -cert ./test_user_50${INDEX}_cert.pem -key test_user_50${INDEX}_key.pem -out ./x509up_u501_$INDEX -pwstdin 
  if [ $? -ne 0 ]; then
    echo "Error creating the proxy" 
    break #do not exit, could be a temporary problem
  fi
  cp --reply=yes ./x509up_u501_$INDEX /home/dteamdteampilot$INDEX
  if [ $? -ne 0 ]; then
    echo "Error copying the proxy" 
    exit 1
  fi
  echo "User proxy ./x509up_u501_$INDEX succesfully created and stored in pilot user account" 
  echo "Let's get the AFS token too then!"
  echo "Using get_afs_token script: $get_afstoken_script" 
  afs_pass=`cat $afs_pass_file`
  expect $get_afstoken_script $afs_user $afs_pass
  if [ $? -ne 0 ]; then
    echo "Error getting AFS token" 
    break #do not exit, could be a temporary problem
  fi
  echo "AFS token got" 
  echo "Let's rest now" 
done

