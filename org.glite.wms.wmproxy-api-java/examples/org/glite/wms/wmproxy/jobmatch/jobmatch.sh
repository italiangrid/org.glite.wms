
#############################################
#
#	WMProxyJobListMatchTest:
#
#	input parameters:
#		- p1 = endpoint URL
#		- p2 = delegation ID
#		- p3 = proxy file pathname
#               - p4 = path to JDL file
#
##############################################


top=../../../../../../../
top_src=../../../../../
package=org.glite.wms.wmproxy.jobmatch
class=WMProxyJobListMatchTest

#
# The following jar files are only needed to build the source of the Test class:
#       - glite-wms-jdlj.jar
#       - classad.jar
# The source of WMProxy API doesn't need them !!!
#


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
	${top}repository/jclassads/2.2/share/classad.jar \
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
p4=$4

# launching the test...
# ------------------------
set -x
java -classpath ${classpath} ${package}.${class} ${p1} ${p2} ${p3} ${p4}


