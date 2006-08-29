#!/bin/bash                                                                                 #
# Date: 12/07/2006                                                                          #
# Author: Maria Alandes Pradillo e-mail: maria.alandes.pradillo@cern.ch                     #
# Description of the test: Add 1 member in 1 group                                          #
#                                                                                           #
# Input Parameters:                                                                         #
#       - VO name                                                                           #
#       - VO name or group name                                                             #
#       - User certificate file path                                                        #
#                                                                                           #
# Requisites:                                                                               #
#	- VOMS server                                                                       # 
#       - At least 1 configured VO                                                          #
#       - At least 1 user                                                                   #
#                                                                                           #
# Dependencies:                                                                             #
#	- test_ora_VOMS_InstallServer.sh                                                    #                                                   
#	- test_ora_VO_CreateSingleVO.sh                                                     #
#       - test_USER_CreateSingleUser.sh                                                     #
#       - test_GROUP_CreateSingleGroup.sh                                                   #
#                                                                                           #
#############################################################################################
#                                                                                           #
# Note on security issues: the host and user certificates used by this test are dummy ones  #
# provided by the test utils package.                                                       #
#                                                                                           #
#############################################################################################


voms-admin --vo=$1 add-member $2 $3

if [ $? ]; 
then
  echo $? ' TEST : Assign role, has succeeded'
else
  echo $? ' TEST : Assign role, has failed'
fi

