#!/bin/bash                                                                                 #
# Date: 12/07/2006                                                                          #
# Author: Maria Alandes Pradillo e-mail: maria.alandes.pradillo@cern.ch                     #
# Description of the test: Remove 1 VO                                                      #
#                                                                                           #
# Input Parameters:                                                                         #
#       - VO name                                                                           #
#                                                                                           #
# Requisites:                                                                               #
#	- VOMS server                                                                       # 
#       - 1 configured VO                                                                   #
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

cd /opt/glite/etc/config/scripts
./glite-voms-server-config.py --vo=$1 --remove 

if [ $? ]; 
then
  echo 'TEST : Remove VO, has succeeded'
else
  echo 'TEST : Remove VO, has failed'
fi

