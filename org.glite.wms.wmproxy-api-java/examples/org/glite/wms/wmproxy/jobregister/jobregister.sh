
#############################################
#
#	WMProxyJobRegisterTest
#
#	input parameters:
#		- p1 = endpoint URL
#		- p2 = proxy file pathname
#		- p3 = delegation ID
#		- p4 = path to JDL file
#
##############################################


top=../../../../../../../
top_src=../../../../../
package=org.glite.wms.wmproxy.jobregister
class=WMProxyJobRegisterTest

#
# The following jar files are only needed to build this Test class:
#       - glite-wms-jdlj.jar 
#       - classad.jar 
# The WMProxy API class doesn't need them !!!
#


## AXIS ===============
line=`more ../wmp-api-java-test.cfg | grep AXIS_LOC`
AXIS_LOC=${line##*=}

## API_JAVA  ==============
line=`more ../wmp-api-java-test.cfg | grep WMP_API_JAVA`
WMP_API_JAVA=${line##*=}

## JDL_JAVA  ==============
line=`more ../wmp-api-java-test.cfg | grep JDL_API_JAVA`
JDL_API_JAVA=${line##*=}

## CLASSAD  ==============
line=`more ../wmp-api-java-test.cfg | grep CLASSAD_LOC`
CLASSAD=${line##*=}

## SECURITY_TRUSTMANAGER ==============
line=`more ../wmp-api-java-test.cfg | grep SECURITY_TRUSTMANAGER`
SECURITY_TRUSTMANAGER=${line##*=}

## UTIL_JAVA ============ ==============
line=`more ../wmp-api-java-test.cfg | grep SECURITY_UTIL_JAVA`
SECURITY_UTIL_JAVA=${line##*=}

## BOUNCYCASTLE ==============
line=`more ../wmp-api-java-test.cfg | grep BOUNCYCASTLE`
BOUNCYCASTLE=${line##*=}



for p in \
	${top_src} \
        ${AXIS_LOC}/*.jar \
        ${WMP_API_JAVA} \
	${SECURITY_TRUSTMANAGER} \
	${SECURITY_UTIL_JAVA} \
	${BOUNCYCASTLE} \
        ${JDL_API_JAVA} \
        ${CLASSAD} 
	
do
	if ! printenv JSS_CLASSPATH | grep -q "${p}"; then
		if [ -n "${classpath}" ]; then
			classpath="${classpath}":"${p}"
		else
			classpath="${p}"
		fi
	fi
done
#input parameters
#----------------
p1=$1
p2=$2
p3=$3
p4=$4

# launching the test...
# ------------------------
CMD="${package}.${class} ${p1} ${p2} ${p3} ${p4}"
echo "java ${CMD}"
java -classpath ${classpath} ${CMD}


