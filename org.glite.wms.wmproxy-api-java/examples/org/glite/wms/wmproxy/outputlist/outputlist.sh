
#############################################
#
#	WMProxyOutputListTest
#
#	input parameters:
#		- p1 = endpoint URL
#		- p2 = proxy file pathname
#		- p3 = jobID
#
##############################################


top=../../../../../../../
top_src=../../../../../
package=org.glite.wms.wmproxy.outputlist
class=WMProxyOutputListTest

AXIS=`more ../axis.cfg`
AXIS_LOC=${top}$AXIS
echo "axis in : "$AXIS_LOC


for p in \
	${top_src} \
	${top}stage/share/java/glite-wms-wmproxy-api-java.jar \
	${top}stage/share/java/glite-security-trustmanager.jar \
	${top}stage/share/java/glite-security-util-java.jar \
	${top}repository/bcprov-jdk14/1.22/share/jars/bcprov-jdk14-122.jar \
	$AXIS_LOC/*.jar 
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

# launching the test...
# ------------------------
java -classpath ${classpath} ${package}.${class} ${p1} ${p2} ${p3}


