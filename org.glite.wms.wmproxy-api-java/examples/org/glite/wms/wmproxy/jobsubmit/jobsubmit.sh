
#############################################
#
#	WMProxyJobSubmitTest:
#
#	input parameters:
#		- p1 = endpoint URL
#		- p2 = proxy file pathname
#               - p3 = delegation ID
#               - p4 = path to JDL file
#
##############################################


top=../../../../../../../
top_src=../../../../../
package=org.glite.wms.wmproxy.jobsubmit
class=WMProxyJobSubmitTest

AXIS=`more ../axis.cfg`
AXIS_LOC=${top}$AXIS
echo "axis in : "$AXIS_LOC


for p in \
	${top_src} \
	${top}stage/share/java/glite-wms-wmproxy-api-java.jar \
	${top}repository/jclassads/2.1/share/classad.jar \
	${top}stage/share/java/glite-wms-jdlj.jar \
	${top}stage/share/java/glite-security-trustmanager.jar \
	${top}stage/share/java/glite-security-util-java.jar \
	${top}stage/share/java/glite-security-delegation-java.jar \
	${top}repository/bcprov-jdk14/1.22/share/jars/bcprov-jdk14-122.jar \
	 ${top}repository/bcprov-jdk14/1.22/share/jars/jce-jdk13-122.jar \
	${top}repository/axis/1.2/share/axis-1.2/webapps/axis/WEB-INF/lib/axis.jar \
	${top}repository/axis/1.2/share/axis-1.2/webapps/axis/WEB-INF/lib/jaxrpc.jar \
	${top}repository/axis/1.2/share/axis-1.2/webapps/axis/WEB-INF/lib/log4j-1.2.8.jar \
	${top}repository/axis/1.2/share/axis-1.2/webapps/axis/WEB-INF/lib/commons-logging.jar \
	${top}repository/axis/1.2/share/axis-1.2/webapps/axis/WEB-INF/lib/commons-discovery.jar \
	${top}repository/axis/1.2/share/axis-1.2/webapps/axis/WEB-INF/lib/saaj.jar
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


