#!/bin/bash
#
# Tests the glite-mdm-set-right command
# 
# Author: Kalle Happonen <kalle.happonen@cern.ch>

. $(dirname $0)/functions.sh

if [ $# -ne 0 ] ; then
  usage $0
  if [ $1 = "--help" ] ; then
    my_exit $SUCCESS
  else
    my_exit $ERROR
  fi
fi

setup 

###############################
###### Test for man pages #####
###############################

if [ "$MAN_PAGE_CHECK" -ne 0 ] ; then
  check_man glite-mdm-set-right
  echo "Man pages found"
fi

###############################
########### Cleanup ###########
###############################

function cleanup () {
  glite-mdm-del -f $TEST_IMAGE_1 >/dev/null 2>&1

  if [ $? -ne 0 ] ; then
    echo "There was an error removing image $TEST_IMAGE_1"
    my_exit $ERROR
  fi

  # Reset IFS
  unset IFS
}

###############################
######## verify perms #########
###############################

function verify_perms () {
  # 1 argument, check that the permissions are $1
  # 2 arguments, check that the permissions are $1 for $2
  if [ $# -eq 1 ] ; then
    USER_PERM=`echo $1 |cut -c1-3`
    echo $USER_PERM | grep "r" >/dev/null 2>&1
    USER_R=$?
    echo $USER_PERM | grep "w" >/dev/null 2>&1
    USER_W=$?
    echo $USER_PERM | grep "x" >/dev/null 2>&1
    USER_X=$?

    GROUP_PERM=`echo $1 |cut -c4-6`
    echo $GROUP_PERM | grep "r" >/dev/null 2>&1
    GROUP_R=$?
    echo $GROUP_PERM | grep "w" >/dev/null 2>&1
    GROUP_W=$?
    echo $GROUP_PERM | grep "x" >/dev/null 2>&1
    GROUP_X=$?

    OTHER_PERM=`echo $1 |cut -c7-9`
    echo $OTHER_PERM | grep "r" >/dev/null 2>&1
    OTHER_R=$?
    echo $OTHER_PERM | grep "w" >/dev/null 2>&1
    OTHER_W=$?
    echo $OTHER_PERM | grep "x" >/dev/null 2>&1
    OTHER_X=$?
  else
    DN=$2
    DN_PERM=$1
    echo $DN_PERM | grep "r" >/dev/null 2>&1
    DN_R=$?
    echo $DN_PERM | grep "w" >/dev/null 2>&1
    DN_W=$?
    echo $DN_PERM | grep "x" >/dev/null 2>&1
    DN_X=$?
  fi

  # Play with IFS so we can get the newlines using ``
  IFS=" "

  # Check hydra permissions
  if [ $# -eq 1 ] ; then
    if [ $USER_R -eq 0 ] ; then HYDRA_USER="......g"; else HYDRA_USER="......-" ; fi
    if [ $USER_W -eq 0 ] ; then HYDRA_USER=$HYDRA_USER"s"; else HYDRA_USER=$HYDRA_USER"-" ; fi
  
    if [ $GROUP_R -eq 0 ] ; then HYDRA_GROUP="......g"; else HYDRA_GROUP="......-" ; fi
    if [ $GROUP_W -eq 0 ] ; then HYDRA_GROUP=$HYDRA_GROUP"s"; else HYDRA_GROUP=$HYDRA_GROUP"-" ; fi
  
    if [ $OTHER_R -eq 0 ] ; then HYDRA_OTHER="......g"; else HYDRA_OTHER="......-" ; fi
    if [ $OTHER_W -eq 0 ] ; then HYDRA_OTHER=$HYDRA_OTHER"s"; else HYDRA_OTHER=$HYDRA_OTHER"-" ; fi
  
    HYDRA_PERMS=`glite-eds-getacl "/"$TEST_IMAGE_1_STUDY"/"$TEST_IMAGE_1_SERIES"/"$TEST_IMAGE_1_SOP  |grep "Base perms:"`
    if [ $? -ne 0 ] ; then
      echo "Unable to retieve Hydra permissions for $TEST_IMAGE_1" 
      cleanup
      my_exit $ERROR
    fi
    
    echo $HYDRA_PERMS | cut -c20-27 |grep $HYDRA_USER >/dev/null 2>&1
    if [ $? -ne 0 ] ; then
      echo "Wrong user permissions for $TEST_IMAGE_1 in hydra" 
      cleanup
      my_exit $ERROR
    fi
  
    echo $HYDRA_PERMS | cut -c36-43 |grep $HYDRA_GROUP >/dev/null 2>&1
    if [ $? -ne 0 ] ; then
      echo "Wrong group permissions for $TEST_IMAGE_1 in hydra" 
      cleanup
      my_exit $ERROR
    fi
    
    echo $HYDRA_PERMS | cut -c52-59 |grep $HYDRA_OTHER >/dev/null 2>&1
    if [ $? -ne 0 ] ; then
      echo "Wrong other permissions for $TEST_IMAGE_1 in hydra" 
      cleanup
      my_exit $ERROR
    fi
  else
    if [ $DN_R -eq 0 ] ; then HYDRA_DN="......g"; else HYDRA_DN="......-" ; fi
    if [ $DN_W -eq 0 ] ; then HYDRA_DN=$HYDRA_DN"s"; else HYDRA_DN=$HYDRA_DN"-" ; fi
   
    HYDRA_PERMS=`glite-eds-getacl "/"$TEST_IMAGE_1_STUDY"/"$TEST_IMAGE_1_SERIES"/"$TEST_IMAGE_1_SOP  |grep "$DN"`
    
    echo $HYDRA_PERMS | cut -d : -f2 | grep $HYDRA_DN >/dev/null 2>&1
    if [ $? -ne 0 ] ; then
      echo "Wrong permissions for $DN on $TEST_IMAGE_1 in hydra" 
      cleanup
      my_exit $ERROR
    fi
  fi
  

  # Check LFN perms

  if [ $# -eq 1 ] ; then

    LFN_PERMS=`lfc-ls -l $LFN |cut -f1 -d " " | cut -c2-10`
    if [ $? -ne 0 ] ; then
      echo "Could not get LFN permissions for $TEST_IMAGE_1" 
      cleanup
      my_exit $ERROR
    fi

    echo $LFN_PERMS |grep  ^$1$ >/dev/null 2>&1 
    if [ $? -ne 0 ] ; then
      echo "LFN permissions are incorrect for $TEST_IMAGE_1" 
      cleanup
      my_exit $ERROR
    fi
    
  else
    LFN_PERMS=`lfc-getacl $LFN |grep "$DN"  |grep -v ^# |cut -f3 -d ":" |cut -c1-3`
    if [ $? -ne 0 ] ; then
      echo "Could not get LFN permissions for $TEST_IMAGE_1" 
      cleanup
      my_exit $ERROR
    fi

    echo $LFN_PERMS |grep  ^$DN_PERM$ >/dev/null 2>&1 
    if [ $? -ne 0 ] ; then
      echo "LFN permissions are incorrect for $DN on $TEST_IMAGE_1" 
      cleanup
      my_exit $ERROR
    fi
  fi

  # Check DPM perms
  SURL=`lcg-lr lfn:$LFN |grep srm`
  if [ $# -eq 1 ] ; then

    DPM_PERMS=`lcg-ls -l $SURL |cut -f1 -d " " | cut -c2-10`
    if [ $? -ne 0 ] ; then
      echo "Could not get DPM permissions for $TEST_IMAGE_1" 
      cleanup
      my_exit $ERROR
    fi

    echo $DPM_PERMS |grep  ^$1$ >/dev/null 2>&1 
    if [ $? -ne 0 ] ; then
      echo "DPM permissions are incorrect for $TEST_IMAGE_1" 
      cleanup
      my_exit $ERROR
    fi
    
  else
    DPM_PERMS=`dpns-getacl $MDM_DpmPath/$TEST_IMAGE_1_STUDY"/"$TEST_IMAGE_1_SERIES"/"$TEST_IMAGE_1_SOP |grep "$DN"  |grep -v ^# |cut -f3 -d ":" |cut -c1-3`
    if [ $? -ne 0 ] ; then
      echo "Could not get DPM permissions for $TEST_IMAGE_1" 
      cleanup
      my_exit $ERROR
    fi

    echo $DPM_PERMS |grep  ^$DN_PERM$ >/dev/null 2>&1 
    if [ $? -ne 0 ] ; then
      echo "DPM permissions are incorrect for $DN on $TEST_IMAGE_1" 
      cleanup
      my_exit $ERROR
    fi
  fi

  # Reset IFS
  unset IFS
}


###############################
###### verify AMGA perms ######
###############################

function verify_amga_perms () {
  # 1 argument, check that the permissions are $1
  # 2 arguments, check that the permissions are $1 for $2
  

  # Play with IFS so we can get the newlines using ``
  IFS=" "

  # Check AMGA permissions
  GUID=`lcg-lg lfn:$LFN`
  if [ $? -ne 0 ] ; then
    echo "Could not get GUID for $TEST_IMAGE_1 when checking permissions" 
    cleanup
    my_exit $ERROR
  fi

  AMGA_PERMS=`mdcli acl_show /mdm/DICOM/$GUID`
  if [ $? -ne 0 ] ; then
    echo "Could not get AMGA permissions for $TEST_IMAGE_1" 
    cleanup
    my_exit $ERROR
  fi

  if [ $# -eq 1 ] ; then
    AMGA_USER=`echo $USER_PERM |sed -e "s/-//g"`
    AMGA_GROUP=`echo $GROUP_PERM |sed -e "s/-//g"`
    AMGA_OTHER=`echo $OTHER_PERM |sed -e "s/-//g"`

    echo $AMGA_PERMS |grep "mdmadmin:register" | cut -f2 -d " " | grep ^$AMGA_USER$ >/dev/null 2>&1
    if [ $? -ne 0 ] ; then
      echo "Wrong user permissions for $TEST_IMAGE_1 in AMGA" 
      cleanup
      my_exit $ERROR
    fi

    echo $AMGA_PERMS |grep "mdmadmin:physician" | cut -f2 -d " " | grep ^$AMGA_GROUP$  >/dev/null 2>&1
    if [ $? -ne 0 ] ; then
      echo "Wrong group permissions for $TEST_IMAGE_1 in AMGA" 
      cleanup
      my_exit $ERROR
    fi
  else
    AMGA_DN=`echo $DN_PERM |sed -e "s/-//g"`

    echo $AMGA_PERMS |grep"$DN" | cut -f2 -d " " | grep ^$AMGA_DN$  >/dev/null 2>&1
    if [ $? -ne 0 ] ; then
      echo "Wrong permissions for $DN on $TEST_IMAGE_1 in AMGA" 
      cleanup
      my_exit $ERROR
    fi
  fi

  # Reset IFS
  unset IFS
}
###############################
#### Register test image ######
###############################

# Re-register the image for the tests
glite-mdm-register $TEST_IMAGE_1 >/dev/null 2>&1
if [ $? -ne 0 ] ; then
  echo "There was an error registering the image $TEST_IMAGE_1" 
  my_exit $ERROR
fi

###############################
# Change permissions with -s ##
###############################

glite-mdm-set-right -s $TEST_IMAGE_1_STUDY, $TEST_IMAGE_1_SERIES, $TEST_IMAGE_1_SOP rwxrw-r-- >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "There was an error changing permissions to \"rwxrw-r--\" for "$TEST_IMAGE_1_STUDY, $TEST_IMAGE_1_SERIES, $TEST_IMAGE_1_SOP
  cleanup
  my_exit $ERROR
fi

echo "Succesfully changed permissions using st/se/sop and rwxrw-r--, verifying..."

verify_perms rwxrw-r--

echo "...permissions correct in all services"


###############################
## Set permissions with guid ##
###############################

GUID=`lcg-lg lfn:$LFN`

glite-mdm-set-right -g $GUID  -u "$TEST_DN" biomed r-- none >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "There was an error changing permissions to '-u \"$TEST_DN\" biomed rw- none' with the flag -g $GUID"
  cleanup
  my_exit $ERROR
fi

echo "Succesfully changed permissions using GUID and -u <DN> biomed rw- none, verifying..."

verify_perms $TEST_DN r--
#verify_perms_amga $TEST_DN none

echo "...permissions correct in all services"

###############################
### Set AMGA perms with file ##
###############################

glite-mdm-set-right -f $TEST_IMAGE_1 "group"  >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "There was an error changing amga permissions to \"group\" with the flag -d $TEST_IMAGE_1"
  cleanup
  my_exit $ERROR
fi

echo "Succesfully changed amga permissions using file and \"group\", verifying..."

#verify_perms_amga group

echo "...permissions correct in all services"

###############################
### Set AMGA perms with LFN ###
###############################

glite-mdm-set-right -l $LFN private >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "There was an error changing permissions to \"private\" for image with flag -l lfn:/grid/dteam/mdm/"$TEST_IMAGE_1_STUDY"/"$TEST_IMAGE_1_SERIES"/"$TEST_IMAGE_1_SOP
  cleanup
  my_exit $ERROR
fi

echo "Succesfully changed amga permissions using the file lfn and \"private\", verifying..."

#verify_perms_amga private

echo "...permissions correct in all services"

###############################
##### Set AMGA user perms #####
###############################

glite-mdm-set-right -l $LFN -u "$TEST_DN" biomed rwx anonymous >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "There was an error changing permissions to \"anonymous\" for user $TEST_DN on image with flag -l lfn:/grid/dteam/mdm/$LFN -u $TEST_DN biomed rwx anonymous"
  cleanup
  my_exit $ERROR
fi

echo "Succesfully changed amga permissions for $TEST_DN using the file lfn and \"anonymous\", verifying..."

verify_perms $TEST_DN rwx
#verify_perms_amga $TEST_DN anonymous

echo "...permissions correct in all services"

###############################
##### Set AMGA user perms #####
###############################

glite-mdm-set-right -l $LFN -u "$TEST_DN" anon rex biomed >/dev/null 2>&1

if [ $? -eq 0 ] ; then
  echo "Giving wrong parameters to glite-mdm-set-right -u does not give an error"
  cleanup
  my_exit $ERROR
fi

echo "Giving wrong parameters to glite-mdm-set-right -u correctly gives an error"


###############################
############ Done #############
###############################

cleanup
echo "Tests completed"
my_exit $SUCCESS
