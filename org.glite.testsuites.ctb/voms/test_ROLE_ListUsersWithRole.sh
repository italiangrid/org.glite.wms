#!/bin/bash                                                                                 #
# Date: 12/07/2006                                                                          #
# Author: Maria Alandes Pradillo e-mail: maria.alandes.pradillo@cern.ch                     #
# Description of the test: List user with roles                                             #
#                                                                                           #
# Input Parameters:                                                                         #
#       - VO name                                                                           #
#       - VO name or group name                                                             #
#       - Role                                                                              #
#                                                                                           #
# Requisites:                                                                               #
#	- VOMS server                                                                       # 
#       - At least 1 configured VO                                                          #
#       - At least 1 user                                                                   #
#       - At least 1 role                                                                   #
#       - At least 1 role assigned to the user                                              #
#                                                                                           #
# Dependencies:                                                                             #
#	- test_ora_VOMS_InstallServer.sh                                                    #                                                   
#	- test_ora_VO_CreateSingleVO.sh                                                     #
#       - test_USER_CreateSingleUser.sh                                                     #
#       - test_USER_CreateSingleRole.sh                                                     #
#       - test_ROLE_AssignRoleUser.sh                                                       #
#                                                                                           #
#############################################################################################
#                                                                                           #
# Note on security issues: the host and user certificates used by this test are dummy ones  #
# provided by the test utils package.                                                       #
#                                                                                           #
#############################################################################################


voms-admin --vo=$1 list-users-with-role $2 $3

if [ $? ]; 
then
  echo 'TEST : List user role, has succeeded'
else
  echo 'TEST : List user role, has failed'
fi

