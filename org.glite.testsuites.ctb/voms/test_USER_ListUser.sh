#!/bin/bash                                                                                 #
# Date: 12/07/2006                                                                          #
# Author: Maria Alandes Pradillo e-mail: maria.alandes.pradillo@cern.ch                     #
# Description of the test: List user                                                        #
#                                                                                           #
# Input Parameters:                                                                         #
#       - VO name                                                                           #
#                                                                                           #
# Requisites:                                                                               #
#	- VOMS server                                                                       # 
#       - 1 configured VO                                                                   #
#       - at least 1 user registered in the VO                                              #
#                                                                                           #
# Dependencies:                                                                             #
#	- test_ora_VOMS_InstallServer.sh                                                    #                                                   
#	- test_ora_VO_CreateSingleVO.sh                                                     #
#       - test_USER_CreateSingleUser.sh                                                     #
#                                                                                           #
#############################################################################################
#                                                                                           #
# Note on security issues: the host and user certificates used by this test are dummy ones  #
# provided by the test utils package.                                                       #
#                                                                                           #
#############################################################################################


voms-admin --vo=$1 list-users 

if [ $? ]; 
then
  echo 'TEST : List user, has succeeded'
else
  echo 'TEST : List user, has failed'
fi

