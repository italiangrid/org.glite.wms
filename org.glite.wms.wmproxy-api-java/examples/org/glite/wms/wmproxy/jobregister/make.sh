top=../../../../../../../
top_src=../../../../../
package=org.glite.wms.wmproxy.jobregister
class=WMProxyJobRegisterTest

java_file=${class}.java
class_file=${class}.class

#rm class file
# ------------------
if test -f ${class_file} ; then
        echo "Removing "${class_file} 
        rm -fr ${class_file}
fi

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

for p in \
        ${top_src} \
        ${AXIS_LOC}/jaxrpc.jar \
	${WMP_API_JAVA} \
        ${JDL_API_JAVA} \
	${CLASSAD} 
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

