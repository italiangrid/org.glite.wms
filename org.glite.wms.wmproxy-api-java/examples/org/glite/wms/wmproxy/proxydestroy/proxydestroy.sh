
#############################################
#
#	WMProxyDestroyTest
#
#	input parameters:
#		- p1 = endpoint URL
#		- p2 = proxy file pathname
#		- p3 = CAs pathname (optional)
#
##############################################


top=../../../../../../../
top_src=../../../../../
package=org.glite.wms.wmproxy.proxydestroy
class=WMProxyDestroyProxyTest


## AXIS ===============
line=`more ../wmp-api-java-test.cfg | grep AXIS_LOC`
AXIS_LOC=${line##*=}

## API_JAVA ==============
line=`more ../wmp-api-java-test.cfg | grep WMP_API_JAVA`
WMP_API_JAVA=${line##*=}

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
	${WMP_API_JAVA} \
        ${UI_API_JAVA} \
	${SECURITY_TRUSTMANAGER} \
	${SECURITY_UTIL_JAVA} \
	${BOUNCYCASTLE} \
	${AXIS_LOC}/*.jar
do
	if ( (! test -f $p) && (! test -d $p ) ) ; then
		echo "classpath error - path not found: " + $p
		exit
	fi

	if ! printenv $classpath | grep -q "${p}"; then
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

# launching the test...
# ------------------------
CMD="${package}.${class} ${p1} ${p2} ${p3}"
echo "java ${CMD}"
java -classpath ${classpath} ${CMD}



