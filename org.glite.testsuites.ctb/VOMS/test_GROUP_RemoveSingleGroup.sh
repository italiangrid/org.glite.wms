#!/bin/bash                                                                                 #
# Date: 12/07/2006                                                                          #
# Author: Maria Alandes Pradillo e-mail: maria.alandes.pradillo@cern.ch                     #
# Description of the test: Remove 1 group                                                   #
#                                                                                           #
# Input Parameters:                                                                         #
#       - VO name                                                                           #
#       - Group name                                                                        #
#                                                                                           #
# Requisites:                                                                               #
#	- VOMS server                                                                       # 
#       - At least 1 configured VO                                                          #
#       - At least 1 group                                                                  #
#                                                                                           #
# Dependencies:                                                                             #
#	- test_ora_VOMS_InstallServer.sh                                                    #                                                   
#	- test_ora_VO_CreateSingleVO.sh                                                     #
#       - test_GROUP_RemoveSingleGroup.sh                                                   #
#                                                                                           #
#############################################################################################
#                                                                                           #
# Note on security issues: the host and user certificates used by this test are dummy ones  #
# provided by the test utils package.                                                       #
#                                                                                           #
#############################################################################################


voms-admin --vo=$1 delete-group $2

if [ $? ]; 
then
  echo $? ' TEST : Delete group, has succeeded'
else
  echo $? ' TEST : Delete group, has failed'
fi

