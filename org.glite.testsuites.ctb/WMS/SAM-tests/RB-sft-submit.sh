#!/bin/bash
#****** RB/RB-sft-submit
# NAME
# RB-sft-submit -
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


function exitonerr {
    rm -f $log testjob.jid 
    exit $SAME_ERROR            
}

. $SAME_SENSOR_HOME/config.sh

log="RB-sft-submit.log"
centralCE=`cat centralCE`
inMaintenance=`cat inMaintenance`

if [ "$1" == "--publish" ] ; then
    if [ ! -e testjob.jid ] ; then 
        exit 0
    fi

    jobStatus=`$JOB_STATUS_CMD -i testjob.jid `
    if [ "$?" != "0" ]; then
        echo "<h2>Edg-job-status</h2>" >> $log
        echo "<pre>" >> $log
        exec 3>&1 4>&2 1>>$log 2>&1
        set -x
        $JOB_STATUS_CMD  -i  testjob.jid
        set +x
        exec 1>&- 1>&3 2>&4 3>&- 4>&-
        echo "</pre>" >> $log
        cat $log        
        echo "Job status failed! Check the Resource Broker"
        exitonerr
    fi

    if [ "`echo $jobStatus | grep -E 'Done\ \(Success\)'`" ] ; then # job ended succesfuly     :D
        echo "<h2>Edg-job-status</h2>" >> $log
        echo "<pre>" >> $log
        exec 3>&1 4>&2 1>>$log 2>&1
        set -x
        $JOB_STATUS_CMD  -i  testjob.jid
        RESULT=$?
        set +x
        exec 1>&- 1>&3 2>&4 3>&- 4>&-
        echo "</pre>" >> $log
        if [ "$RESULT" != "0" ]; then  # if failed       :(
            cat $log        
            echo "Job status failed! Check the Resource Broker"
            exitonerr
        fi

        echo "<h2>Edg-job-get-output</h2>" >> $log
        echo "<pre>" >> $log
        exec 3>&1 4>&2 1>>$log 2>&1
        set -x
        $JOB_OUTPUT_CMD   -c edg_wl_ui_cmd_var-submit.conf -i  testjob.jid
        RESULT=$?
        set +x
        exec 1>&- 1>&3 2>&4 3>&- 4>&-
        echo "</pre>" >> $log

        if [ "$RESULT" != "0" ]; then  # if failed :(
            cat $log        
            echo "Job get output failed! Check the Resource Broker"
            exitonerr
        fi

        grep "Error: UI_CREATE_DIR" $log >/dev/null
        RESULT=$?
        if [ "$RESULT" == "0" ] ; then
            echo "Job get output failed due to edg-job-get-output bug - problem related with software"
        fi 
    
        if [ "$RESULT" != "0" ] ; then
            echo "<h2>Job output</h2>" >> $log
            OUT=`cat ${USER}_*/testjob.out`
            echo "<pre>" >> $log
            echo "> cat ${USER}_*/testjob.out" >> $log
            echo $OUT >> $log
            echo "</pre>" >> $log
            if [ "$OUT" == "SAM" ]; then 
                    echo "Job output correct" >> $log   
                    exitonerr=0;
            else
                    echo "Job output incorrect"
                    exitonerr=1;
            fi 
        fi

        echo "<h2>Edg-job-get-logging-info</h2>" >> $log
        echo "<pre>" >> $log
        exec 3>&1 4>&2 1>>$log 2>&1
        set -x
        $JOB_LOGGING_INFO_CMD -i testjob.jid
        RESULT=$?
        set +x
        exec 1>&- 1>&3 2>&4 3>&- 4>&-
        echo "</pre>" >> $log

        if [ "$RESULT" != "0" ]; then  # if failed :(
            cat $log        
            echo "Job logging info failed! Check the Resource Broker"
            exitonerr
        fi

        cat $log   
        if [ $exitonerr == "1" ]; then
        exitonerr
       fi

        rm -f $log testjob.jid 
        exit $SAME_OK                                                                                                                           
    fi

    if [ "`echo $jobStatus | grep -E 'Aborted|Done\ \(Failed\)'`" ] ; then
        echo "<h2>Edg-job-status</h2>" >> $log
        echo "<pre>" >> $log
        exec 3>&1 4>&2 1>>$log 2>&1
        set -x
        $JOB_STATUS_CMD  -i  testjob.jid
        RESULT=$?
        set +x
        exec 1>&- 1>&3 2>&4 3>&- 4>&-
        echo "</pre>" >> $log
        if [ "$RESULT" != "0" ]; then  # if failed       :(
            cat $log        
            echo "Job status failed! Check the Resource Broker"
            exitonerr
        fi

        echo "<h2>Edg-job-get-logging-info</h2>" >> $log
        echo "<pre>" >> $log
        exec 3>&1 4>&2 1>>$log 2>&1
        set -x
        $JOB_LOGGING_INFO_CMD -i testjob.jid
        RESULT=$?
        set +x
        exec 1>&- 1>&3 2>&4 3>&- 4>&-
        echo "</pre>" >> $log

        if [ "$RESULT" != "0" ]; then  # if failed    :(
            cat $log        
            echo "Job logging info failed! Check the Resource Broker"
            exitonerr
        fi
        cat $log     
        rm -f $log testjob.jid 
        exit $SAME_OK
    fi
                                                                    
    exit 0  # job not finished, get rest
else  # submit
    if [ -e testjob.jid ] ; then
       # job exists, do nothing, get rest
        exit 0
 fi

    siteName=$1
    nodeName=$2
    
    # clean your room pumpkin'
    rm -f $log testjob.jid edg_wl_ui-submit.conf edg_wl_ui_cmd_var-submit.conf testjob-submit.jdl   
    rm -rf ${USER}_*

    echo "<h2>Generating JDL file:</h2>" >> $log
    sed  -e "s/<centralCE>/$centralCE/g" $SAME_SENSOR_HOME/testjob.jdl > testjob-submit.jdl
    echo "<pre>" >> $log
    cat testjob-submit.jdl >> $log
    echo "</pre>" >> $log
    echo "<h2>Generating edg_wl_ui_cmd_var.conf file:</h2>" >> $log
    sed  -e "s/<nodeName>/$nodeName/g" -e "s/<SAME_VO>/$SAME_VO/g" -e "s|<SAME_SENSOR_WORK>|$SAME_SENSOR_WORK|g"  $SAME_SENSOR_HOME/edg_wl_ui_cmd_var.conf > edg_wl_ui_cmd_var-submit.conf
    echo "<pre>" >> $log
    cat edg_wl_ui_cmd_var-submit.conf >> $log
    echo "</pre>" >> $log
    echo "<h2>Generating edg_wl_ui.conf file:</h2>" >> $log
    sed  -e "s/<nodeName>/$nodeName/g" -e "s/<SAME_VO>/$SAME_VO/g" $SAME_SENSOR_HOME/edg_wl_ui.conf > edg_wl_ui-submit.conf
    echo "<pre>" >> $log
    cat edg_wl_ui-submit.conf >> $log
    echo "</pre>" >> $log

    echo "<h2>Edg-job-submit</h2>" >> $log
    echo "<pre>" >> $log
    exec 3>&1 4>&2 1>>$log 2>&1
    set -x
    $JOB_SUBMIT_CMD -c edg_wl_ui_cmd_var-submit.conf --config-vo edg_wl_ui-submit.conf -o testjob.jid testjob-submit.jdl
    set +x
    exec 1>&- 1>&3 2>&4 3>&- 4>&-
    echo "</pre>" >> $log
    if [ ! -e testjob.jid ] ; then
        echo "Job submission failed! Check the Resource Broker" >> $log
        echo "summary: SubmitError" >> $log
        cat $log
        exit $SAME_ERROR
    fi
    exit 0
fi

