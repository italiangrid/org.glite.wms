
top=../../../../../../../
top_src=../../../../../
package=org.glite.wms.wmproxy.sbulkdesturi
class=WMProxySandboxDestURITest

AXIS=`more ../axis.cfg`
AXIS_LOC=${top}$AXIS
echo "axis in : "$AXIS_LOC

for p in \
	${top_src} \
	${top}stage/share/java/glite-wms-wmproxy-api-java.jar \
	$AXIS_LOC/axis.jar \
	$AXIS_LOC/jaxrpc.jar
do
	if ! printenv JSS_CLASSPATH | grep -q "${p}"; then
		if [ -n "${classpath}" ]; then
			classpath="${classpath}":"${p}"
		else
			classpath="${p}"
		fi
	fi
done

set -x

javac -classpath ${classpath} ${class}.java


