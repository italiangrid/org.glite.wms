#!/bin/sh

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

if [ $INDEX -eq 10 ]; then
  usernum=510
else
  usernum=50${INDEX}
fi

echo "using test user $usernum"
#cp --reply=yes $users_certs_dir/test_user_50${INDEX}_cert.pem ./
cp --reply=yes $users_certs_dir/test_user_${usernum}_cert.pem ./
cp --reply=yes $users_certs_dir/test_user_${usernum}_key.pem ./
chown root ./test_user_${usernum}_cert.pem
chown root ./test_user_${usernum}_key.pem

export PATH="$PATH:/opt/glite/bin/"

echo "Creating proxy file x509up_u501_${usernum}"  
echo "test" | glite-voms-proxy-init -q --voms dteam -cert ./test_user_${usernum}_cert.pem -key test_user_${usernum}_key.pem -out ./x509up_u501_${usernum} -pwstdin 
#true
if [ $? -ne 0 ]; then
  echo "Error creating the proxy" 
  exit 1
fi
cp --reply=yes ./x509up_u501_${usernum} /home/dteamdteampilot${INDEX}/x509up_u501_${INDEX}
if [ $? -ne 0 ]; then
  echo "Error copying the proxy" 
  exit 1
fi
echo "User proxy ./x509up_u501_${usernum} succesfully created and stored in pilot user account" 

echo "Entering the loop" 
while [ 1 ] 
do
  sleep 4h
  echo "Creating proxy file x509up_u501_${usernum}" 
  echo "test" | glite-voms-proxy-init -q --voms dteam -cert ./test_user_${usernum}_cert.pem -key test_user_${usernum}_key.pem -out ./x509up_u501_${usernum} -pwstdin 
  if [ $? -ne 0 ]; then
    echo "Error creating the proxy" 
    break #do not exit, could be a temporary problem
  fi
  cp --reply=yes ./x509up_u501_${usernum} /home/dteamdteampilot${INDEX}/x509up_u501_${INDEX}
  if [ $? -ne 0 ]; then
    echo "Error copying the proxy" 
    exit 1
  fi
  echo "User proxy ./x509up_u501_${usernum} succesfully created and stored in pilot user account" 
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

