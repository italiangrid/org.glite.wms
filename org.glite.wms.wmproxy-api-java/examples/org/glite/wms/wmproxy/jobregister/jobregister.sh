
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

AXIS=`more ../axis.cfg`
AXIS_LOC=${top}$AXIS
echo "axis in : "$AXIS_LOC

for p in \
	${top_src} \
	${top}stage/share/java/glite-wms-wmproxy-api-java.jar \
	${top}stage/share/java/glite-security-trustmanager.jar \
	${top}stage/share/java/glite-security-util-java.jar \
	${top}repository/bcprov-jdk14/1.22/share/jars/bcprov-jdk14-122.jar \
	$AXIS_LOC/axis.jar \
	$AXIS_LOC/jaxrpc.jar \
	$AXIS_LOC/log4j-1.2.8.jar \
	$AXIS_LOC/commons-logging.jar \
	$AXIS_LOC/commons-discovery.jar \
	$AXIS_LOC/saaj.jar \
	${top}stage/share/java/glite-wms-jdlj.jar \
        ${top}repository/jclassads/2.1/share/classad.jar 

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
java -classpath ${classpath} ${package}.${class} ${p1} ${p2} ${p3} ${p4}


