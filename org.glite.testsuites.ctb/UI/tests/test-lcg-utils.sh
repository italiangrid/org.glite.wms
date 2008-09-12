#!/bin/sh
#set -x
#=============================================================================
#
# Test suite for lcg-utils
# 
#=============================================================================

#=============================================================================
# Prints the usage
#=============================================================================
usage(){
cat <<EOF 1>&2
Usage: $0 -v | --vo vo [--dpm hostname] [--castor hostname] [--classic hostname] [--dcache hostname] 
EOF
}

#=============================================================================
# Parse input arguments and set global vairables 
#=============================================================================
OPTS=$(getopt -o "v" -l "castor:,dcache:,dpm:,classic:,vo:" -n $0 -- "$@")
if [ $? != 0 ]; then 
    usage 
    exit 1 
fi
eval set -- $OPTS
while [ $# -gt 0 ]; do
  case "$1" in
      -v|--vo) VO=$2; shift 2;;
      --castor) CASTOR=$2; shift 2;;
      --dcache) DCACHE=$2; shift 2;;
      --dpm) DPM=$2; shift 2;;
      --classic) CLASSIC=$2; shift 2;;
      --) shift ; break ;;      
      *) echo "Error: Invalid Argument $1" 1>&2; exit 1 ;;
  esac  
done

if [ -z ${VO} ]; then
    echo "Error: VO not set." 1>&2
    usage
    exit 1
fi

export VO=${VO}

#=============================================================================
# Create Source:Destination pairs
#=============================================================================
if [ ! -z ${CASTOR} ] && [ ! -z ${DPM} ]; then  
    pairs="${pairs} ${CASTOR}:${DPM}"
    pairs="${pairs} ${DPM}:${CASTOR}"
fi

if [ ! -z ${CASTOR} ] && [ ! -z ${DCACHE} ]; then  
    pairs="${pairs} ${CASTOR}:${DCACHE}"
    pairs="${pairs} ${DCACHE}:${CASTOR}"
fi

if [ ! -z ${CASTOR} ] && [ ! -z ${CACHE} ]; then  
    pairs="${pairs} ${CASTOR}:${CLASSIC}"
    pairs="${pairs} ${CLASSIC}:${CASTOR}"
fi
if [ ! -z ${DPM} ] && [ ! -z ${DCACHE} ]; then  
    pairs="${pairs} ${DPM}:${DCACHE}"
    pairs="${pairs} ${DCACHE}:${DPM}"
fi
if [ ! -z ${DPM} ] && [ ! -z ${CLASSIC} ]; then  
    pairs="${pairs} ${DPM}:${CLASSIC}"
    pairs="${pairs} ${CLASSIC}:${DPM}"
fi
if [ ! -z ${DCACHE} ] && [ ! -z ${CLASSIC} ]; then  
    pairs="${pairs} ${DCACHE}:${CLASSIC}"
    pairs="${pairs} ${CLASSIC}:${DCACHE}"
fi

if [ -z "${pairs}" ]; then
    echo "Error: Need at least two different Storage Services." 1>&2
    usage
    exit 1
fi

#=============================================================================
# Check Grid Proxy
#=============================================================================
grid-proxy-info > /dev/null
if [ $? -gt 0 ]; then
    exit 1
fi

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
# Test preperation
#=============================================================================
uni_pid=$$
uni_time=`date +%s`
name="$uni_pid$uni_time"

export LFN="lfn:/grid/${VO}/$name"
export ALIAS="${LFN}-alias"
export LOCAL_FILE=/etc/redhat-release
export TEMP_FILE=/tmp/${name}
export RETVAL=0

failed=no

#=============================================================================
# Do test for each pair
#=============================================================================

echo "Testing lcg-utils for VO ${VO} ... "
echo $pairs

for pair in ${pairs}; do
    source=$(echo ${pair} | cut -d ":" -f1)
    dest=$(echo ${pair} | cut -d ":" -f2)
    export SOURCE=${source}
    export DEST=${dest}
    echo
    echo "#################################################"
    echo "Testing source=${SOURCE} and destination=${DEST}."
    echo "#################################################"

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
    
    SURL=$(echo ${OUTPUT} | grep -i -E -o 'sfn.*|srm.*' )
    
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
    
#The following call is meaningful only in case of SRM service
    if [ "$SOURCE" != "$CLASSIC" ];then
      command="lcg-sd ${SURL} ${REQID} 0 "
      message="Running sd command"
      run_command "${command}" "${message}"
    fi 
    
    command="lcg-del -v -a --vo ${VO} ${LFN}"
    message="Running delete command"
    run_command "${command}" "${message}"
    
    command="lcg-cr -v --vo dteam -l ${LFN} file:${LOCAL_FILE} -P /grid/dteam/${TEMP_FILE} -d ${DEST}"
    message="Running copy and register with a specified path"
    run_command "${command}" "${message}"
    
    command="lcg-del -v -a --vo ${VO} ${LFN}"
    message="Running delete command"
    run_command "${command}" "${message}"
done


if [ ${RETVAL} -gt 0 ]; then
    echo "#################"
    echo "# -TEST FAILED- #"
    echo "#################"
else
    echo "#################"
    echo "# -TEST PASSED- #"
    echo "#################"
fi

exit ${RETVAL}

																	     
