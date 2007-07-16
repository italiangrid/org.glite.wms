#!/bin/bash

VO=${1:-dteam}		 
USER=${1:-dteam091}      #Change for different qsub users
DATE=`date -Iminutes`
OUTCE="IDs/$DATE"_".CE.out
OUTBDII="IDs/$DATE"_".BDII.out

TIME=`date -Iminutes`
LA=`sed -e 's/^\([^ ]*\).*\/\([^ ]*\).*/\1 \2/' /proc/loadavg`


SGE_QMASTER=`ps auxw|grep sge_qmaster|awk '{print $3" "$4}'|awk 'BEGIN{cpu=0;sz=0}{cpu+=$1;sz+=$2}END{print cpu" "sz}'`
SGE_SCHEDD=`ps auxw|grep sge_schedd|awk '{print $3" "$4}'|awk 'BEGIN{cpu=0;sz=0}{cpu+=$1;sz+=$2}END{print cpu" "sz}'`
BLPARSER=`ps auxw|grep globus-job-manager-script.pl|awk '{print $3" "$4}'|awk 'BEGIN{cpu=0;sz=0}{cpu+=$1;sz+=$2}END{print cpu" "sz}'`


JOBS=`qstat -q $VO|awk 'BEGIN {r=0;o=0}/'"$VO"'/{if ($5 == "r") r++ ;if ($5 ~ /[qweht]/) o++} END {print r" "o}'`
COMPLETED=`qstat -s z -u $USER | awk 'BEGIN { cont=0 }{ cont++; if ( cont > 3 ) print $1}' | wc -l`

[ $? -eq 0 ] || COMPLETED="0"

echo -e "$TIME\t$LA\t$BLPARSER\t$SGE_SCHEDD\t$SGE_QMASTER\t$JOBS\t$COMPLETED" 
