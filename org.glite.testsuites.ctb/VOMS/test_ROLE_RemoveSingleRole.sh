#!/bin/bash                                                                                 #
# Date: 12/07/2006                                                                          #
# Author: Maria Alandes Pradillo e-mail: maria.alandes.pradillo@cern.ch                     #
# Description of the test: Remove 1 role                                                    #
#                                                                                           #
# Input Parameters:                                                                         #
#       - VO name                                                                           #
#       - Role name                                                                         #
#                                                                                           #
# Requisites:                                                                               #
#	- VOMS server                                                                       # 
#       - 1 configured VO                                                                   #
#                                                                                           #
# Dependencies:                                                                             #
#	- test_ora_VOMS_InstallServer.sh                                                    #                                                   
#	- test_ora_VO_CreateSingleVO.sh                                                     #
#       - test_ROLE_CreateSingleRole.sh                                                     #
#                                                                                           #
#############################################################################################
#                                                                                           #
# Note on security issues: the host and user certificates used by this test are dummy ones  #
# provided by the test utils package.                                                       #
#                                                                                           #
#############################################################################################


voms-admin --vo=$1 delete-role $2

if [ $? ]; 
then
  echo $? ' TEST : Delete role, has succeeded'
else
  echo $? ' TEST : Delete role, has failed'
fi

