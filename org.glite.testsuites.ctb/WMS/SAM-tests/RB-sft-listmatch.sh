#!/bin/bash
#****** RB/RB-sft-listmatch
# NAME
# RB-sft-listmatch -
#
# AUTHOR
#
# Malgorzata Krakowian
#
# LAST UPDATED
#
#
# LANGUAGE
#
# bash 
#
# SOURCE


. $SAME_SENSOR_HOME/config.sh

log="RB-sft-listmatch.log"
centralCE=`cat centralCE`
inMaintenance=`cat inMaintenance`

if [ "$1" != "--publish" ]; then
    siteName=$1
    nodeName=$2
    rm -f $log edg_wl_ui-listmatch.conf edg_wl_ui_cmd_var-listmatch.conf testjob-listmatch.jdl
    echo "<h2>Generating JDL file:</h2>" >> $log
    grep -v Requirements $SAME_SENSOR_HOME/testjob.jdl > testjob-listmatch.jdl
    echo "<pre>" >> $log
    cat testjob-listmatch.jdl >> $log
    echo "</pre>" >> $log
    echo "<h2>Generating edg_wl_ui_cmd_var.conf file:</h2>" >> $log
    sed  -e "s/<nodeName>/$nodeName/g" -e "s/<SAME_VO>/$SAME_VO/g" -e "s|<SAME_SENSOR_WORK>|$SAME_SENSOR_WORK|g"  $SAME_SENSOR_HOME/edg_wl_ui_cmd_var.conf > edg_wl_ui_cmd_var-listmatch.conf
    echo "<pre>" >> $log
    cat edg_wl_ui_cmd_var-listmatch.conf >> $log
    echo "</pre>" >> $log
    echo "<h2>Generating edg_wl_ui.conf file:</h2>" >> $log
    sed  -e "s/<nodeName>/$nodeName/g" -e "s/<SAME_VO>/$SAME_VO/g" $SAME_SENSOR_HOME/edg_wl_ui.conf > edg_wl_ui-listmatch.conf
    echo "<pre>" >> $log
    cat edg_wl_ui-listmatch.conf >> $log
    echo "</pre>" >> $log

    echo "<h2>Edg-job-list-match</h2>" >> $log
    echo "<pre>" >> $log
    exec 3>&1 4>&2 1>>$log 2>&1
    set -x
    $JOB_LISTMATCH_CMD -c edg_wl_ui_cmd_var-listmatch.conf --config-vo edg_wl_ui-listmatch.conf testjob-listmatch.jdl
    RESULT=$?
    set +x
    exec 1>&- 1>&3 2>&4 3>&- 4>&-
    echo "</pre>" >> $log
    if [ "$RESULT" != "0" ] ; then
    echo "Job listmatch failed! Check the Resource Broker" >> $log
    echo "summary: listmatchError" >> $log
    cat $log
        rm -f $log edg_wl_ui-listmatch.conf edg_wl_ui_cmd_var-listmatch.conf testjob-listmatch.jdl
    exit $SAME_ERROR
    fi
    grep failure $log >/dev/null
    RESULT=$?
    if [ "$RESULT" == "0" ] ; then
    echo "Job listmatch failed! Check the Resource Broker" >> $log
    echo "summary: listmatchError" >> $log
    cat $log
        rm -f $log edg_wl_ui-listmatch.conf edg_wl_ui_cmd_var-listmatch.conf testjob-listmatch.jdl
    exit $SAME_ERROR
    fi
 
    cat $log
    rm -f $log edg_wl_ui-listmatch.conf edg_wl_ui_cmd_var-listmatch.conf testjob-listmatch.jdl
    exit $SAME_OK 
fi
exit 0

