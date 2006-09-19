top=../../../../../../../
top_src=../../../../../
package=org.glite.wms.wmproxy.puproxy
class_1=WMProxyPutProxyTest
class_2=WMProxyGrstProxyTest
class_3=WMProxyNewProxyTest

java_file_1=${class_1}.java
class_file_2=${class_1}.class


java_file_2=${class_2}.java
class_file_2=${class_2}.class

java_file_3=${class_3}.java
class_file_3=${class_3}.class

#rm class file
# ------------------
for i in\
	${class_file_1} \
	${class_file_2} \
	${class_file_3}
do
		if test -f ${i} ; then
			echo "Removing "${i}
			rm -fr ${i}
		fi
done

## AXIS ===============
line=`more ../wmp-api-java-test.cfg | grep AXIS_LOC`
AXIS_LOC=${line##*=}

## API_JAVA  ==============
line=`more ../wmp-api-java-test.cfg | grep WMP_API_JAVA`
WMP_API_JAVA=${line##*=}


for p in \
        ${top_src} \
        ${WMP_API_JAVA} \
        ${AXIS_LOC} /axis.jar \
        ${AXIS_LOC}/jaxrpc.jar
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

javac -classpath ${classpath} ${java_file_1} ${java_file_2} ${java_file_3}

