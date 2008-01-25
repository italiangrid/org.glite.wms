#!/bin/sh
#set -x
#=============================================================================
#
# Test suite for lcg-utils
# 
#=============================================================================

#=== FUNCTION ================================================================
# Prints the usage
#=============================================================================
function usage(){
    echo "Usage: $0 -v <voname> -s <firstSE> -d <secondSE>"
}

#=== FUNCTION ================================================================
# Parse input arguments and set global vairables 
#=============================================================================
#function parse_arguments(){
    while getopts "v:s:d:" arg ; do
	case "$arg" in
	    v) vo="$OPTARG" ;;
	    s) source="$OPTARG" ;;
	    d) dest="$OPTARG" ;;
	    -) break ;;
	    ?) usage
	    exit 1 ;;
	esac
    done
    if [ "x${vo}" == "x"  -o "x${source}" == "x" -o "x${dest}" == "x" ]; then
	usage
	exit 1
    fi
    export VO=${vo}
    export SOURCE=${source}
    export DEST=${dest}
#}

#=== FUNCTION ================================================================
# test preperation
#=============================================================================
function prepare(){
    uni_pid=$$
    uni_time=`date +%s`
    name="$uni_pid$uni_time"
    
    export LFN="lfn:/grid/${VO}/$name"
    export ALIAS="${LFN}-alias"
    export LOCAL_FILE=/etc/redhat-release
    export TEMP_FILE=/tmp/${name}
}

#=== FUNCTION ================================================================
# Checks for a valid proxy.
#=============================================================================
function check_proxy(){
    $(grid-proxy-info >/dev/null 2>&1)
    if [ $? -gt 0 ]; then
	echo "Error: Couldn't find a valid proxy."
	exit 1
    fi
}

#=== FUNCTION ================================================================
# Runs Test Command
#=============================================================================
function run_command() {
    command=$1
    message=$2
    echo -n "${message} ... "
    output=$(eval "${command}" 2>&1)
    if [ $? -gt 0 ]; then
	echo "Failed"
	echo "${command}" >&2
	echo "${output}" >&2
	export RETVAL=1
	return 1
    fi
    echo "OK"
    export OUTPUT=${output}
    return 0
}
    
#=============================================================================
# Start Test
#=============================================================================

prepare
check_proxy

export RETVAL=0
echo "Testing lcg-utils for VO ${VO} ... "

command="lcg-cr -v --vo ${VO} -l ${LFN} file:${LOCAL_FILE} -d ${SOURCE}"
message="Running copy and register command"
run_command "${command}" "${message}"

command="lcg-lg -v --vo ${VO} ${LFN}"
message="Running list GUID by LFN command"
run_command "${command}" "${message}"

GUID=${OUTPUT}
export GUID

command="lcg-cp -v --vo ${VO} ${GUID} file:${TEMP_FILE}"
message="Running download file command using GUID"
run_command "${command}" "${message}"

command="diff -q ${TEMP_FILE} ${LOCAL_FILE}"
message="Comparing the content with the original file"
run_command "${command}" "${message}"

command="lcg-cp -v --vo ${VO} ${LFN} file:${TEMP_FILE}"
message="Running download file command using LFN"
run_command "${command}" "${message}"

command="diff -q ${TEMP_FILE} ${LOCAL_FILE}"
message="Comparing the content with the original file"
run_command "${command}" "${message}"

command="lcg-lr -v --vo ${VO} ${LFN}"
message="Running list replica command"
run_command "${command}" "${message}"

SURL=$(echo ${OUTPUT})

command="lcg-cp -n 1 -v ${SURL} file:///dev/null"
message="Running copy with streams"
run_command "${command}" "${message}"

command="lcg-lg -v --vo ${VO} ${SURL}"
message="Running list GUID by SURL command"
run_command "${command}" "${message}"

command="test  ${GUID} = ${OUTPUT} "
message="Checking that the GUIDs match"
run_command "${command}" "${message}"

command="lcg-ls ${SURL}"
message="Running ls command"
run_command "${command}" "${message}"

command="lcg-aa  -v --vo ${VO} ${GUID} ${ALIAS}"
message="Running create alias command"
run_command "${command}" "${message}"

command="lcg-la  -v --vo ${VO} ${GUID} | grep alias"
message="Running list alias command"
run_command "${command}" "${message}"

command="echo ${OUTPUT} | grep alias "
message="Checking that the alias exists"
run_command "${command}" "${message}"

command="lcg-ra  -v --vo ${VO} ${GUID} ${ALIAS}"
message="Running remove alias command"
run_command "${command}" "${message}"

command="echo ${OUTPUT} | grep -v alias "
message="Checking that the alias has been removed"
run_command "${command}" "${message}"

command="lcg-rep -v --vo ${VO} ${LFN} -d ${DEST}"
message="Running replicate command"
run_command "${command}" "${message}"

command="lcg-lr --vo ${VO} ${LFN}"' | grep '"${DEST}"
message="Checking that the replica exists"
run_command "${command}" "${message}"

command="lcg-uf --vo dteam ${GUID} ${SURL}"
message="Running unregister file command"
run_command "${command}" "${message}"

command="lcg-rf --vo dteam -g ${GUID} ${SURL}"
message="Running register file command"
run_command "${command}" "${message}"

command="lcg-gt -v $SURL gsiftp"
message="Running get TURL command"
run_command "${command}" "${message}"

REQID=$(echo ${OUTPUT} | awk '{print $2;}')

command="lcg-sd ${SURL} ${REQID} 0 0"
message="Running sd command"
run_command "${command}" "${message}"

command="lcg-del -v -a --vo ${VO} ${LFN}"
message="Running delete command"
run_command "${command}" "${message}"

command="lcg-cr -v --vo dteam -l ${LFN} file:${LOCAL_FILE} -P /grid/dteam/${TEMP_FILE} -d ${DEST}"
message="Running copy and register with a specified path"
run_command "${command}" "${message}"

command="lcg-del -v -a --vo ${VO} ${LFN}"
message="Running delete command"
run_command "${command}" "${message}"

if [ ${RETVAL} -gt 0 ]; then
    echo "Tests Failed "
else
    echo "Tests Passed "
fi

exit ${RETVAL}

																	     
