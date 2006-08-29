#!/bin/bash                                                                                 #
# Date: 12/07/2006                                                                          #
# Author: Maria Alandes Pradillo e-mail: maria.alandes.pradillo@cern.ch                     #
# Description of the test: Create 1 ACL                                                     #
#                                                                                           #
# Input Parameters:                                                                         #
#       - VO name                                                                           #
#       - Role or VO name or group name                                                     #
#       - Allow or Deny                                                                     #
#       - Operation                                                                         #
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
#                                                                                           #
#############################################################################################
#                                                                                           #
# Note on security issues: the host and user certificates used by this test are dummy ones  #
# provided by the test utils package.                                                       #
#                                                                                           #
#############################################################################################


voms-admin --vo=$1 add-acl-entry $2 $3 $4 $5

if [ $? ]; 
then
  echo $? ' TEST : Create ACL, has succeeded'
else
  echo $? ' TEST : Create ACL, has failed'
fi

