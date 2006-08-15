#!/bin/bash                                                                                 #
# Date: 12/07/2006                                                                          #
# Author: Maria Alandes Pradillo e-mail: maria.alandes.pradillo@cern.ch                     #
# Description of the test: Single Gridmap file generation for 1 VO                          #
# Requisites:                                                                               #
#	- VOMS server                                                                       # 
#       - 1 VO called atlas                                                                 #
#       - lxb1418_gridmap.conf                                                              #
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
#                                                                                           # 
# 04/08/2006: This script has to be improved to be able to configure the gridmapfile        #
# configuration for any VO and any host.                                                    #
#############################################################################################


wget http://egee-jra1-testing.web.cern.ch/egee-jra1-testing/testbed/config_files/testsuites/lxb1418_gridmap.conf
mv lxb1418_gridmap.conf /opt/edg/etc/. 
/opt/edg/sbin/edg-mkgridmap --conf /opt/edg/etc/lxb1418_gridmap.conf --safe > mkgridmap_users
voms-admin --vo=atlas list-users > voms_users
sed '1d' voms_users > temp
num_grid=`grep -c CN mkgridmap_users`
num_voms=`grep -c CN temp`

if [ num_voms==num_grid ]; 
then
  echo 'TEST : gridmap file has been successfully generated'
else
  echo 'TEST : gridmap file generation has failed'
fi

rm -rf voms_users
rm -rf mkgridmap_users
rm -rf temp
