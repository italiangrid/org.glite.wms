#!/bin/bash                                                                                 #
# Date: 12/07/2006                                                                          #
# Author: Maria Alandes Pradillo e-mail: maria.alandes.pradillo@cern.ch                     #
# Description of the test: List default ACL                                                 #
#                                                                                           #
# Input Parameters:                                                                         #
#       - VO name                                                                           #
#       - Role name or group name or VO name                                                #
#                                                                                           #
# Requisites:                                                                               #
#	- VOMS server                                                                       # 
#       - At least 1 configured VO                                                          #
#       - At least 1 default ACL                                                            #
#                                                                                           #
# Dependencies:                                                                             #
#	- test_ora_VOMS_InstallServer.sh                                                    #                                                   
#	- test_ora_VO_CreateSingleVO.sh                                                     #
#       - test_ACL_CreateSingleDefaultACL.sh                                                #
#                                                                                           #
#############################################################################################
#                                                                                           #
# Note on security issues: the host and user certificates used by this test are dummy ones  #
# provided by the test utils package.                                                       #
#                                                                                           #
#############################################################################################


voms-admin --vo=$1 get-default-acl $2 

if [ $? ]; 
then
  echo 'TEST : List default ACL, has succeeded'
else
  echo 'TEST : List default ACL, has failed'
fi

