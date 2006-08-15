#!/bin/bash                                                                          #
# Date: 12/07/2006                                                                   #
# Author: Maria Alandes Pradillo e-mail: maria.alandes.pradillo@cern.ch              #
# Description of the test: Installs a VOMS server using APT                          #
# Requisites: None                                                                   #
# Inputs: None                                                                       #
#                                                                                    #
#                                                                                    #  
######################################################################################

# Install Oracle instant client and JAVA RPMs
echo 'Fetch oracle-instantclient-basic-10.2.0.1-1.i386.rpm...'
wget http://egee-jra1-testing.web.cern.ch/egee-jra1-testing/NMI_tests/oracle-instantclient-basic-10.2.0.1-1.i386.rpm
echo 'Fetch oracle-instantclient-sqlplus-10.2.0.1-1.i386.rpm...'
wget http://egee-jra1-testing.web.cern.ch/egee-jra1-testing/NMI_tests/oracle-instantclient-sqlplus-10.2.0.1-1.i386.rpm
echo 'Fetch j2sdk-1_4_2_08-linux-i586.rpm...'
wget http://egee-jra1-testing.web.cern.ch/egee-jra1-testing/NMI_tests/j2re-1_4_2_08-linux-i586.rpm

echo 'Install oracle-instantclient-basic-10.2.0.1-1.i386.rpm...'
rpm -ivh oracle-instantclient-basic-10.2.0.1-1.i386.rpm
echo 'Install oracle-instantclient-sqlplus-10.2.0.1-1.i386.rpm...'
rpm -ivh oracle-instantclient-sqlplus-10.2.0.1-1.i386.rpm
echo 'Install j2sdk-1_4_2_08-linux-i586.rpm...'
rpm -ivh j2re-1_4_2_08-linux-i586.rpm 


# Install VOMS using the certification APT repository
echo 'rpm http://lxb2042.cern.ch/gLite/APT/R3.0-cert/ rhel30 externals Release3.0 updates' > /etc/apt/sources.list.d/glite.list
echo 'rpm http://linuxsoft.cern.ch  cern/slc30X/i386/apt  os updates extras' > /etc/apt/sources.list.d/cern.list
echo 'rpm-src http://linuxsoft.cern.ch  cern/slc30X/i386/apt  os updates extras' >> /etc/apt/sources.list.d/cern.list
apt-get clean
apt-get update
apt-get install --yes glite-VOMS_oracle

 
