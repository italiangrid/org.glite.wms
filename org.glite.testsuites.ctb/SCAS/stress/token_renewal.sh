#!/bin/sh

function usage() {
echo "Usage: $0 "
}

if [ $# -ne 0 ]; then
  usage
  exit 1
fi

source setup_test.cfg

echo "Using get_afs_token script: $get_afstoken_script" 
afs_pass=`cat $afs_pass_file`
expect $get_afstokenlocal_script $afs_user $afs_pass
if [ $? -ne 0 ]; then
  echo "Error getting AFS token" 
  break #do not exit, could be a temporary problem
fi
echo "AFS token got" 

echo "Entering the loop" 
while [ 1 ] 
do
  sleep 4h
  echo "Using get_afs_token script: $get_afstoken_script" 
  afs_pass=`cat $afs_pass_file`
  expect $get_afstokenlocal_script $afs_user $afs_pass
  if [ $? -ne 0 ]; then
    echo "Error getting AFS token" 
    break #do not exit, could be a temporary problem
  fi
  echo "AFS token got" 
  echo "Let's rest now" 
done

exit 0
