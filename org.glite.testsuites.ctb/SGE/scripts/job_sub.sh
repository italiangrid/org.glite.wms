#!/bin/bash

DATE=`date -Iminutes`

SITE=${1:-cern}
NUM=${2:-20}
JOB=${3:-sa3-200.jdl} 
WMS=$4

IDS="IDs/$DATE"_"$SITE"
OUT="IDs/$DATE"_"$SITE"_"$NUM".out
STATUS="IDs/$DATE"_"$SITE"_"$NUM".status

#mkdir IDs

if [ $SITE == "uoa" ];then
        WMS="https://ctb06.gridctb.uoa.gr:7443/glite_wms_wmproxy_server"
elif [ $SITE == "cern" ];then
        WMS="https://lxb2032.cern.ch:7443/glite_wms_wmproxy_server"
        #WMS="https://wms01.egee.cesga.es:7443/glite_wms_wmproxy_server"
elif [ $SITE == "cy" ];then
        WMS="https://wmslb201.grid.ucy.ac.cy:7443/glite_wms_wmproxy_server"
fi

DELAY=1

echo "$DATE"> $OUT
echo "$DATE"> $STATUS
time for i in `seq 1 $NUM`; do 
        #glite-wms-job-submit -a -o $IDS -c /opt/cesga/lcg-sa3/glite/etc/dteam/glite_wms.conf -e $WMS $JOB
	glite-wms-job-submit -a -o $IDS -e $WMS $JOB
        sleep $DELAY
done>>$OUT 2>&1 &

while true
do
        SEC=`date +%S`
        if [ $SEC -ge "59" -o $SEC -le "1" ];then
                JOBS="`glite-wms-job-status --noint --logfile /dev/null -i $IDS |awk '
                BEGIN {a["Done"]     =a["Ready"]    =a["Running"]=a["Scheduled"]=0;
                       a["Undefined"]=a["Submitted"]=a["Waiting"]=a["Cleared"]  =0;
                       a["Aborted"]  =a["Cancelled"]=a["Purged"] =a["Unknown"]  =0;}
                /Current Status:[ ]*([^ ]*)/ {a[$3]+=1;total+=1}
                END {if (total==0) exit 1;
                     for (i in a){ 
                          printf("%s=%s\t",i,a[i]);}
                     printf("\n");}'`"
                if [ $? -eq 0 ];then
                        CURRENT_DATE=`date -Iminutes`
                        echo -e "$CURRENT_DATE\t$JOBS" >> $STATUS
                        WAIT_JOBS=`echo "$JOBS" |egrep -qi "(SUBMITTED=[^0]|WAITING=[^0]|READY=[^0]|SCHEDULED=[^0]|RUNNING=[^0])";echo $?`
                        if [ "$WAIT_JOBS" -ne 0 ];then
                                break;
                        fi
OUT="IDs/$DATE"_"$SITE"_"$NUM".out
STATUS="IDs/$DATE"_"$SITE"_"$NUM".status

if [ $SITE == "uoa" ];then
        WMS="https://ctb06.gridctb.uoa.gr:7443/glite_wms_wmproxy_server"
elif [ $SITE == "cern" ];then
        WMS="https://lxb2032.cern.ch:7443/glite_wms_wmproxy_server"
        #WMS="https://wms01.egee.cesga.es:7443/glite_wms_wmproxy_server"
elif [ $SITE == "cy" ];then
        WMS="https://wmslb201.grid.ucy.ac.cy:7443/glite_wms_wmproxy_server"
fi

DELAY=1

echo "$DATE"> $OUT
echo "$DATE"> $STATUS
time for i in `seq 1 $NUM`; do 
        glite-wms-job-submit -a -o $IDS -e $WMS $JOB
        sleep $DELAY
done>>$OUT 2>&1 &

while true
do
        SEC=`date +%S`
        if [ $SEC -ge "59" -o $SEC -le "1" ];then
                JOBS="`glite-wms-job-status --noint --logfile /dev/null -i $IDS |awk '
                BEGIN {a["Done"]     =a["Ready"]    =a["Running"]=a["Scheduled"]=0;
                       a["Undefined"]=a["Submitted"]=a["Waiting"]=a["Cleared"]  =0;
                       a["Aborted"]  =a["Cancelled"]=a["Purged"] =a["Unknown"]  =0;}
                /Current Status:[ ]*([^ ]*)/ {a[$3]+=1;total+=1}
                END {if (total==0) exit 1;
                     for (i in a){ 
                          printf("%s=%s\t",i,a[i]);}
                     printf("\n");}'`"
                if [ $? -eq 0 ];then
                        CURRENT_DATE=`date -Iminutes`
                        echo -e "$CURRENT_DATE\t$JOBS" >> $STATUS
                        WAIT_JOBS=`echo "$JOBS" |egrep -qi "(SUBMITTED=[^0]|WAITING=[^0]|READY=[^0]|SCHEDULED=[^0]|RUNNING=[^0])";echo $?`
                        if [ "$WAIT_JOBS" -ne 0 ];then
                                break;
                        fi

                        sleep 10
                fi
        fi
        sleep 2
done
