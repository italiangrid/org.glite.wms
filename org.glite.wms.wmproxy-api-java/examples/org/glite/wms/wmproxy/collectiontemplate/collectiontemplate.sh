# Copyright (c) Members of the EGEE Collaboration. 2004.
# See http://www.eu-egee.org/partners/ for details on the
# copyright holders.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
# either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


#############################################
#
#	WMProxyGetVersionTest
#
#	input parameters:
#		- p1 = endpoint URL
#		- p2 = proxy file pathname
#		- p3 = configuration file
#               - p4 = CAs files pathname
#
##############################################


top=../../../../../../../
top_src=../../../../../
package=org.glite.wms.wmproxy.collectiontemplate
class=WMProxyCollectionTemplateTest

AXIS=`more ../axis.cfg`
AXIS_LOC=${top}$AXIS
echo "axis in : "$AXIS_LOC
for p in \
	${top_src} \
	${top}stage/share/java/glite-wms-wmproxy-api-java.jar \
	${top}stage/share/java/glite-security-trustmanager.jar \
	${top}stage/share/java/glite-security-util-java.jar \
	${top}repository/bcprov-jdk14/1.22/share/jars/bcprov-jdk14-122.jar \
	${AXIS_LOC}/*.jar
do
	if ( (! test -f $p) && (! test -d $p ) ) ; then
		echo "classpath - path not found: " + $p
		exit
	fi

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
CMD="${package}.${class} ${p1} ${p2} ${p3} ${p4}"
echo "java ${CMD}"
java -classpath ${classpath} ${CMD}


