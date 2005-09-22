top=../../../../../../../
top_src=../../../../../
package=org.glite.wms.wmproxy.collectiontemplate
class=WMProxyCollectionTemplateTest

java_file=${class}.java
class_file=${class}.class

#rm class file
# ------------------
if test -f ${class_file} ; then
        echo "Removing "${class_file} 
        rm -fr ${class_file}
fi

AXIS=`more ../axis.cfg`
AXIS_LOC=${top}$AXIS
echo "axis in : "$AXIS_LOC

for p in \
        ${top_src} \
        ${top}stage/share/java/glite-wms-wmproxy-api-java.jar \
        $AXIS_LOC/axis.jar \
        $AXIS_LOC/jaxrpc.jar \
	$AXIS_LOC/log4j-1.2.8.jar
do
        if ! printenv "${classpath}" | grep -q "${p}"; then
                if [ -n "${classpath}" ]; then
                        classpath="${classpath}":"${p}"
                else
                        classpath="${p}"
                fi
        fi
done

set -x

javac -classpath ${classpath} ${class}.java

