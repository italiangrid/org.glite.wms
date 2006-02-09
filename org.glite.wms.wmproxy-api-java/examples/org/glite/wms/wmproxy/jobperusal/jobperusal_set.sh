
#############################################
#
#	WMProxyEnablePerusalTest
#
#	input parameters:
#		- p1 = endpoint URL
#		- p2 = job-ID
#		- p3 = proxy file pathname
#              - $* = <file1> ... <fileN> [CAs dir pathname]
#
##############################################

top=../../../../../../../
top_src=../../../../../
package=org.glite.wms.wmproxy.jobperusal
class=WMProxyEnablePerusalTest

## AXIS ===============
line=`more ../wmp-api-java-test.cfg | grep AXIS_LOC`
AXIS_LOC=${line##*=}

## API_JAVA  ==============
line=`more ../wmp-api-java-test.cfg | grep WMP_API_JAVA`
WMP_API_JAVA=${line##*=}

## SECURITY_TRUSTMANAGER ==============
line=`more ../wmp-api-java-test.cfg | grep SECURITY_TRUSTMANAGER`
SECURITY_TRUSTMANAGER=${line##*=}

## UTIL_JAVA ============ ==============
line=`more ../wmp-api-java-test.cfg | grep SECURITY_UTIL_JAVA`
SECURITY_UTIL_JAVA=${line##*=}

## DELEGATION_JAVA ============ ==============
line=`more ../wmp-api-java-test.cfg | grep SECURITY_DELEGATION_JAVA`
SECURITY_DELEGATION_JAVA=${line##*=}

## BOUNCYCASTLE ==============
line=`more ../wmp-api-java-test.cfg | grep BOUNCYCASTLE`
BOUNCYCASTLE=${line##*=}

## UI_API_JAVA
line=`more ../wmp-api-java-test.cfg | grep UI_API_JAVA`
UI_API_JAVA=${line##*=}


for p in \
	${top_src} \
	${WMP_API_JAVA} \
	${UI_API_JAVA} \
	${SECURITY_TRUSTMANAGER} \
	${SECURITY_UTIL_JAVA} \
	${BOUNCYCASTLE} \
	${AXIS_LOC}/*.jar
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
p=$*

# launching the test...
# ------------------------
java -classpath ${classpath} ${package}.${class} ${p}


