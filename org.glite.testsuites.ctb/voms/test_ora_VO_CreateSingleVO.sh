#!/bin/bash                                                                                 #
# Date: 12/07/2006                                                                          #
# Author: Maria Alandes Pradillo e-mail: maria.alandes.pradillo@cern.ch                     #
# Description of the test: Configure VOMS with 1 VO called atlas                            #
# Requisites: Oracle account (host, database name, user name and password needed)           #
#                                                                                           #
# Inputs: - SingleVO-vo-list.xml                                                            #
#         - SingleVO-voms-server.xml                                                        #
# This configuration files can be found in                                                  # 
# http://egee-jra1-testing.web.cern.ch/egee-jra1-testing/testbed/config_files/testsuites    #
# REMEMBER to change the host related information in the config files accordingly           #
#                                                                                           #
#############################################################################################
#                                                                                           #
# Note on security issues: the host and user certificates used by this test are dummy ones  #
# provided by the test utils package.                                                       #
#                                                                                           #
#############################################################################################
#                                                                                           #
# 04/08/2006: This script configures a VO called atlas. It would be good to improve the     #
#             script to pass the VO name and other information like database account as a   #
#             parameter. TO BE DONE.                                                        #
#############################################################################################

mydir=$PWD

echo 'Fetch test-utils...'
wget http://egee-jra1-testing.web.cern.ch/egee-jra1-testing/NMI_tests/glite-security-test-utils.noarch.rpm
echo 'Install test-utils...'
rpm -ivh glite-security-test-utils.noarch.rpm

mkdir /etc/grid-security
mkdir /etc/grid-security/certificates
echo 'Copy CAs...'
cp /opt/glite/share/test/certificates/grid-security/certificates/* /etc/grid-security/certificates/.
echo 'Copy host certificates...'
cp /opt/glite/share/test/certificates/grid-security/hostcert.pem /etc/grid-security/hostcert.pem
cp /opt/glite/share/test/certificates/grid-security/hostkey.pem /etc/grid-security/hostkey.pem
chmod 0400 /etc/grid-security/hostkey.pem
echo 'Copy VOMS administrator user certificate...'
cp /opt/glite/share/test/certificates/home/usercert.pem ~/.

echo 'Fetch configuration files...'
wget http://egee-jra1-testing.web.cern.ch/egee-jra1-testing/testbed/config_files/testsuites/SingleVO-vo-list.xml
wget http://egee-jra1-testing.web.cern.ch/egee-jra1-testing/testbed/config_files/testsuites/SingleVO-voms-server.xml
wget http://egee-jra1-testing.web.cern.ch/egee-jra1-testing/testbed/config_files/testsuites/glite-security-utils.cfg.xml
echo 'Copy configuration files...'
cp SingleVO-vo-list.xml /opt/glite/etc/config/vo-list.cfg.xml
cp SingleVO-voms-server.xml /opt/glite/etc/config/glite-voms-server.cfg.xml
cp glite-security-utils.cfg.xml /opt/glite/etc/config/.
cp /opt/glite/etc/config/templates/glite-global.cfg.xml /opt/glite/etc/config/.
cd /opt/glite/etc/config/scripts
#Check configuration
#./glite-voms-server-config.py -c
echo 'Configure VOMS...' 
./glite-voms-server-config.py --configure
echo 'Start VOMS...'
./glite-voms-server-config.py --start

echo 'Check VOMS is installed...'
source /etc/glite/profile.d/glite_setenv.sh
sleep 300
voms-admin --url https://localhost:8443/voms/atlas

if [ $? ]; 
then
  echo 'TEST : Configure VOMS with 1 VO called oracle_vo1, has succeeded'
else
  echo 'TEST : Configure VOMS with 1 VO called oracle_vo1, has failed'
fi

cd $mydir
rm -rf SingleVO-vo-list.xml
rm -rf SingleVO-voms-server.xml
rm -rf glite-security-utils.cfg.xml
rm -rf glite-security-test-utils.noarch.rpm
