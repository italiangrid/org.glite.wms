#. $SAME_HOME/sensors/common/config.sh
. config-common.sh

VER_THRESHOLD=1.7.2-1
if ( lcg_util_ver_ge ${VER_THRESHOLD} ) ; then
	LCG_UTIL_TIMEOUT="--connect-timeout $TIMEOUT_CONNECT --sendreceive-timeout $TIMEOUT_SENDRECEIVE --srm-timeout $TIMEOUT_SRM"
	function echo_config() {
	   echo "<pre>"
       echo "Testing from host: $(hostname -f)"
	   echo "DN: $(voms-proxy-info -identity)"
	   echo "SE timeouts in sec: connect $TIMEOUT_CONNECT, send/receive $TIMEOUT_SENDRECEIVE, SRM $TIMEOUT_SRM"
	   echo "</pre>"
	}
else
	LCG_UTIL_TIMEOUT="-t $TIMEOUT_SENDRECEIVE"
	function echo_config() {
	   echo "<pre>"
	   echo "Testing from host: $(hostname -f)"
	   echo "DN: $(voms-proxy-info -identity)"
	   echo "Connect, send and receive timeouts on SE are set to $TIMEOUT_SENDRECEIVE sec."
	   echo "</pre>"
	}
fi

RET_CODES[$SAME_OK]=OK
RET_CODES[$SAME_WARNING]=WARNING
RET_CODES[$SAME_ERROR]=ERROR
RET_CODES[$SAME_NOTICE]=NOTICE

#FILE_SURL=$SAME_SENSOR_WORK/nodes/$nodeName/testFile.surl
#FILE_ENDPNT=$SAME_SENSOR_WORK/nodes/$nodeName/endpoint.txt
#FILE_SAPATH=$SAME_SENSOR_WORK/nodes/$nodeName/SApath.txt
#FILE_COMM=$SAME_SENSOR_WORK/nodes/$nodeName/comm

FILE_SURL=testFile.surl
FILE_ENDPNT=endpoint.txt
FILE_SAPATH=SApath.txt
FILE_COMM=comm

function check_dep_comm() {
    RETCODE_DEP=`cut -d'|' -f2 $FILE_COMM`
    if [ $RETCODE_DEP -gt $SAME_OK ] ; then
        MSG_COMM="Masked by $(cut -d'|' -f1 $FILE_COMM): $(cut -d'|' -f3 $FILE_COMM)"
        echo "<b>${RET_CODES[${RETCODE_DEP}]}</b>: $MSG_COMM"
        echo "summary: $MSG_COMM"
        exit $RETCODE_DEP
    fi
}

function no_srmv2_endpoint() {
    echo "<p>"
    echo "<p><font color='#cc0000'>WARNING</font>: SRMv2 endpoint not provided</p>"
    echo "</p>"
    echo "summary: SRMv2 endpoint not provided"    
    exit $SAME_WARNING
}
