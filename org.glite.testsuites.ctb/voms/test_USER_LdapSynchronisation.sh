#!/bin/bash                                                                                 #
# Date: 12/07/2006                                                                          #
# Author: Maria Alandes Pradillo e-mail: maria.alandes.pradillo@cern.ch                     #
# Description of the test: ldap synchronisation                                             #
# Requisites:                                                                               #
#	- VOMS server                                                                       # 
#       - 1 VO called atlas                                                                 #
#       - ldap-vo-config.xml                                                                #
#                                                                                           #
# Dependencies:                                                                             #
#	- test_ora_VOMS_InstallServer.sh                                                    #                                                   
#	- test_ora_VO_CreateSingleVO.sh                                                     #
#                                                                                           #
#############################################################################################
#                                                                                           #
# Note on security issues: the host and user certificates used by this test are dummy ones  #
# provided by the test utils package.                                                       #
#                                                                                           #
#############################################################################################


mkdir -f temp
cd temp
echo 'Get ldap-vo-config.xml file...'
wget http://egee-jra1-testing.web.cern.ch/egee-jra1-testing/testbed/config_files/testsuites/ldap-vo-config.xml
mv ldap-vo-config.xml /opt/glite/etc/voms-admin/.
echo 'Restart crontab...'
echo '* * * * *  root run-parts /opt/glite/etc/cron' >> /etc/crontab
crontab /etc/crontab
sleep 60
voms-admin --vo=atlas list-users > VOMS_user_list
ldapsearch -LLL -x -H ldap://grid-vo.nikhef.nl -b "ou=lcgadmin,o=atlas,dc=eu-datagrid,dc=org" > ldap_user_list
num_voms=`wc -l VOMS_user_list | cut -f6 -d' '`
num_ldap=`wc -l ldap_user_list | cut -f6 -d' '`

if [ $? ]; 
then
  echo 'TEST : ldap synchronisation, has succeeded'
else
  echo 'TEST : ldap synchronisation, has failed'
fi
