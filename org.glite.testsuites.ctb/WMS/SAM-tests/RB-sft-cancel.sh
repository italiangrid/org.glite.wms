#!/bin/bash
#****** RB/RB-sft-cancel
# NAME
# RB-sft-cancel -
#
# AUTHOR
#
# Malgorzata Krakowian
#
# LAST UPDATED
#
#  07.08.2007
#
# LANGUAGE
#
# bash 
#
# SOURCE


. $SAME_SENSOR_HOME/config.sh

log="RB-sft-check.log"
centralCE=`cat centralCE`
inMaintenance=`cat inMaintenance`

if [ "$1" != "--publish" ]; then
    siteName=$1
    nodeName=$2
    rm -f $log testjob-cancel.jid edg_wl_ui-cancel.conf edg_wl_ui_cmd_var-cancel.conf testjob-cancel.jdl
    echo "<h2>Generating JDL file:</h2>" >> $log
    sed  -e "s/<centralCE>/$centralCE/g" $SAME_SENSOR_HOME/testjob.jdl > testjob-cancel.jdl
    echo "<pre>" >> $log
    cat testjob-cancel.jdl >> $log
    echo "</pre>" >> $log
    echo "<h2>Generating edg_wl_ui_cmd_var.conf file:</h2>" >> $log
    sed  -e "s/<nodeName>/$nodeName/g" -e "s/<SAME_VO>/$SAME_VO/g" -e "s|<SAME_SENSOR_WORK>|$SAME_SENSOR_WORK|g"  $SAME_SENSOR_HOME/edg_wl_ui_cmd_var.conf > edg_wl_ui_cmd_var-cancel.conf
    echo "<pre>" >> $log
    cat edg_wl_ui_cmd_var-cancel.conf >> $log
    echo "</pre>" >> $log
    echo "<h2>Generating edg_wl_ui.conf file:</h2>" >> $log
    sed  -e "s/<nodeName>/$nodeName/g" -e "s/<SAME_VO>/$SAME_VO/g" $SAME_SENSOR_HOME/edg_wl_ui.conf > edg_wl_ui-cancel.conf
    echo "<pre>" >> $log
    cat edg_wl_ui-cancel.conf >> $log
    echo "</pre>" >> $log

    echo "<h2>Edg-job-submit</h2>" >> $log
    echo "<pre>" >> $log
    exec 3>&1 4>&2 1>>$log 2>&1
    set -x
    $JOB_SUBMIT_CMD -c edg_wl_ui_cmd_var-cancel.conf --config-vo edg_wl_ui-cancel.conf -o testjob-cancel.jid testjob-cancel.jdl
    set +x
    exec 1>&- 1>&3 2>&4 3>&- 4>&-
    echo "</pre>" >> $log
    if [ ! -e testjob-cancel.jid ] ; then
        echo "Job submission failed! Check the Resource Broker" >> $log
       echo "summary: SubmitError" >> $log
       cat $log
           rm -f $log testjob-cancel.jid edg_wl_ui-cancel.conf edg_wl_ui_cmd_var-cancel.conf testjob-cancel.jdl
       exit $SAME_ERROR
    fi


    echo "<h2>Edg-job-cancel</h2>" >> $log
    echo "<pre>" >> $log
    exec 3>&1 4>&2 1>>$log 2>&1
    set -x
    $JOB_CANCEL_CMD  --logfile cancel.log -i testjob-cancel.jid 
    RESULT=$?
    set +x
    exec 1>&- 1>&3 2>&4 3>&- 4>&-
    echo "</pre>" >> $log

    if [ "$RESULT" != "0"  -a ! "`grep "Not allowed to cancel the job" cancel.log`" ] ; then
       echo "Job cancel failed! Check the Resource Broker" >> $log
       echo "summary: CancelError" >> $log
       cat $log
           rm -f $log testjob-cancel.jid edg_wl_ui-cancel.conf edg_wl_ui_cmd_var-cancel.conf testjob-cancel.jdl cancel.log
       exit $SAME_ERROR
    fi
    cat $log
    rm -f $log testjob-cancel.jid edg_wl_ui-cancel.conf edg_wl_ui_cmd_var-cancel.conf testjob-cancel.jdl cancel.log
    exit $SAME_OK 
fi
exit 0

