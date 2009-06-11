#
# Created on Wed Jun 10 16:09:08 CEST 2009
# Gianni DOT Pucciani AT cern DOT ch
#

function createProxies() {
pushd users-certs
echo "Removing old proxies"
rm -f proxy_*

for VOindex in creama creamb dteam ogvt;
do
  if [ $VOindex == "ogvt" ]; then
    vo="org.glite.voms-test"
  else
    vo=$VOindex
  fi
  echo "**Creating proxies for $vo"
  index=`echo first${VOindex}user`
  index=$(( index + 0 )) #put it in number
  echo "*Creating proxies for vo $vo"
  for count in `seq 1 16`;
  do
    echo "Creating proxy for test user ${index}"
    echo $TESTUSERMAGIC | voms-proxy-init --voms $vo -key test_user_${index}_key.pem -cert test_user_${index}_cert.pem  -out proxy_${index} -pwstdin 1>>/dev/null 2>>/dev/null
    if [ $? -ne 0 ]; then
      echo "Error creating proxy"
      exit 1
    fi
    index=$(( index + 1 ))
  done
done

popd

}


function createProxiesDiffRoles() {

pushd users-certs
echo "Removing old proxies"
rm -f proxy_*

for VOindex in creama creamb dteam ogvt;
do
  if [ $VOindex == "ogvt" ]; then
    vo="org.glite.voms-test"
  else
    vo=$VOindex
  fi
  echo "creating proxies for $vo"
  index=`echo first${VOindex}user`
  index=$(( index + 0 )) #put it in number
  echo "**Creating proxies for vo $vo"
  for role in role1 role2 role3 role4;
  do
    echo "*Creating proxies for role $role"   
    for count in `seq 1 4`;
    do
      echo "Creating proxy for test user ${index}"
      echo $TESTUSERMAGIC | voms-proxy-init --voms $vo:/$vo/Role=$role -key test_user_${index}_key.pem -cert test_user_${index}_cert.pem  -out proxy_${index} -pwstdin 1>>/dev/null 2>>/dev/null
      if [ $? -ne 0 ]; then
        echo "Error creating proxy"
        exit 1
      fi
      index=$(( index + 1 ))
    done
  done
done

popd

}

submitJobsAllUsers() {

run=$1

echo "Starting job submission for all users run $run: `date`"

for index in `seq $user1 $userN`;
do
  export X509_USER_PROXY=users-certs/proxy_$index
#  echo "Submitting with test user $index"
  glite-ce-job-submit -a -r $creamEndPoint $jdlFile 2>>/dev/null 1>>jobids/jobid_${index}_${run}.txt
  if [ $? -ne 0 ]; then
    echo "Jobs submission failed"
    echo "Failed" > jobids/jobid_${index}_${run}.txt
  fi
done
echo "Job submission finished: `date`"

}

checkOutput() {

run=$1
echo "Checking Job status run $run: `date`"

failed=$totalfailed
for index in `seq $user1 $userN`;
do
  export X509_USER_PROXY=users-certs/proxy_$index
  glite-ce-job-status `cat jobids/jobid_${index}_${run}.txt` 2>>/dev/null 1>>status/status_${index}
  if [ $? -ne 0 ]; then
    echo "Job status call failed"
    failed=$(( $failed + 1 ))
  else
    cat status/status_${index} | grep DONE-OK >>/dev/null
    if [ $? -ne 0 ]; then
#      echo "Job Failed"
      failed=$(( $failed + 1 ))
    fi
  fi
done
echo "Failed jobs run $run: $failed"
totalfailed=$(( $totalfailed + $failed ))
export totalfailed=$totalfailed

}


