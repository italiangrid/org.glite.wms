#!/bin/bash                                                                                 #
# Date: 12/07/2006                                                                          #
# Author: Maria Alandes Pradillo e-mail: maria.alandes.pradillo@cern.ch                     #
# Description of the test: Create 1 user                                                    #
#                                                                                           #
# Input Parameters:                                                                         #
#       - VO name                                                                           #
#       - User certificate file path                                                        #
#                                                                                           #
# Requisites:                                                                               #
#	- VOMS server                                                                       # 
#       - 1 configured VO                                                                   #
#       - user certificate                                                                  #
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


voms-admin --vo=$1 create-user $2 

if [ $? ]; 
then
  echo 'TEST : Create user, has succeeded'
else
  echo 'TEST : Create user, has failed'
fi

