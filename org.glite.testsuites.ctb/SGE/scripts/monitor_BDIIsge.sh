#!/bin/bash


VO="${1:-dteam}"
CE="${2:-sa3-ce.egee.cesga.es}" #Change for a different machine


DATE=`date -Iminutes`
OUTCE="IDs/$DATE"_".CE.out
OUTBDII="IDs/$DATE"_".BDII.out


GIP_QUERY="/opt/lcg/libexec/lcg-info-dynamic-sge 2>/dev/null"

BDII_PORT=2170
BDII_BASE="mds-vo-name=cesga-sa3,o=grid"
BDII_QUERY="(&(GlueForeignKey=GlueClusterUniqueID=$CE)(GlueCEName=$VO))"
BDII_ATTRS="GlueCEStateRunningJobs GlueCEStateWaitingJobs GlueCEStateTotalJobs"
BDII_QUERY="ldapsearch -h $CE -p $BDII_PORT -x -b $BDII_BASE $BDII_QUERY $BDII_ATTRS"

PRINTJOBS='/RunningJobs/{r=$2}/TotalJobs/{t=$2}/WaitingJobs/{w=$2}END{print r" "t" "w}'

TIME=`date -Iminutes`
LA=`sed -e 's/^\([^ ]*\).*\/\([^ ]*\).*/\1 \2/' /proc/loadavg`


GIP=`/opt/lcg/libexec/lcg-info-dynamic-sge 2>/dev/null|sed -ne "/$VO/,/^$/p"| awk "$PRINTJOBS"`


LDAP=`$BDII_QUERY |awk "$PRINTJOBS"`
[ $? -eq 0 ] || LDAP="0 0 0"

echo -e "$TIME\t$LA\t$GIP\t$LDAP"
