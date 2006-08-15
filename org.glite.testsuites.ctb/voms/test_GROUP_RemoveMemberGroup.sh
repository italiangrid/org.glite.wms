#!/bin/bash                                                                                 #
# Date: 12/07/2006                                                                          #
# Author: Maria Alandes Pradillo e-mail: maria.alandes.pradillo@cern.ch                     #
# Description of the test: Remove 1 user from 1 group                                       #
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
#       - At least 1 group                                                                  #
#       - At least 1 user in the group                                                      #
#                                                                                           #
# Dependencies:                                                                             #
#	- test_ora_VOMS_InstallServer.sh                                                    #                                                   
#	- test_ora_VO_CreateSingleVO.sh                                                     #
#       - test_USER_CreateSingleUser.sh                                                     #
#       - test_GROUP_CreateSingleGroup.sh                                                   #
#       - test_GROUP_AddMemberGroup.sh                                                      #
#                                                                                           #
#############################################################################################
#                                                                                           #
# Note on security issues: the host and user certificates used by this test are dummy ones  #
# provided by the test utils package.                                                       #
#                                                                                           #
#############################################################################################


voms-admin --vo=$1 remove-member $2 $3

if [ $? ]; 
then
  echo $? ' TEST : Remove user from group, has succeeded'
else
  echo $? ' TEST : Remove user from group, has failed'
fi

