#!/bin/bash

#
# bug #42579: [FTS] Only active channels shall be published in BDII
# It has to be run on the FTS server as root
#

if [ $# -ne 1 ]; then
  echo "Usage: $0 <channel>"
  exit 1
fi

export PATH="$PATH:/opt/glite/bin"

channel=$1
host=`hostname`


for setstatus in Active Drain Stopped Inactive;
do
  echo "Setting $channel to $setstatus"
  glite-transfer-channel-set -S  $setstatus $channel
  if [ $? -ne 0 ];then
    echo "channel set failed"
    exit 1
  fi
  echo "Channel status set, sleep 60s"
    sleep 60
  echo "Retrieving channel status"

  GlueServiceStatus=`ldapsearch -x -H ldap://${host}:2170 -b Mds-vo-name=resource,o=Grid -LLL "(&(GlueServiceType=org.glite.ChannelAgent)(GlueServiceName=${channel}_${host}_org.glite.ChannelAgent))" GlueServiceStatus | egrep '(GlueServiceStatus)' | awk '{print \$2}'`

 GlueServiceStatusInfo=`ldapsearch -x -H ldap://${host}:2170 -b Mds-vo-name=resource,o=Grid -LLL "(&(GlueServiceType=org.glite.ChannelAgent)(GlueServiceName=${channel}_${host}_org.glite.ChannelAgent))" GlueServiceStatusInfo | egrep '(GlueServiceStatusInfo)' | awk '{print $2}'`

  case "$setstatus" in
        Active)
            if [ "$GlueServiceStatus" != "OK" ]; then
              echo "Error: GlueServiceStatus=$GlueServiceStatus GlueServiceStatusInfo=$GlueServiceStatusInfo"
              exit 1
            else
              echo "OK: GlueServiceStatus=$GlueServiceStatus GlueServiceStatusInfo=$GlueServiceStatusInfo"
            fi
            ;;
        Drain)
            if [ "$GlueServiceStatus" != "Other" ]; then
              echo "Error: GlueServiceStatus=$GlueServiceStatus GlueServiceStatusInfo=$GlueServiceStatusInfo"
              exit 1
            else 
              echo "OK: GlueServiceStatus=$GlueServiceStatus GlueServiceStatusInfo=$GlueServiceStatusInfo"
            fi
            ;;
        Inactive)
            if [ "$GlueServiceStatus" != "Other" ]; then
              echo "Error: GlueServiceStatus=$GlueServiceStatus GlueServiceStatusInfo=$GlueServiceStatusInfo"
              exit 1
            else 
              echo "OK: GlueServiceStatus=$GlueServiceStatus GlueServiceStatusInfo=$GlueServiceStatusInfo"
            fi
            ;;
        Stopped)
            if [ "$GlueServiceStatus" != "Other" ]; then
              echo "Error: GlueServiceStatus=$GlueServiceStatus GlueServiceStatusInfo=$GlueServiceStatusInfo"
              exit 1
            else
              echo "OK: GlueServiceStatus=$GlueServiceStatus GlueServiceStatusInfo=$GlueServiceStatusInfo"
            fi
            ;;
        *)
           echo "?? $GlueServiceStatusInfo $GlueServiceStatus"
           exit 1
  esac
done

echo "-TEST PASSED-"
exit 0 

