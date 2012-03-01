#!/bin/bash
#
# Copyright (c) Members of the EGEE Collaboration. 2004-2010.
# See http://www.eu-egee.org/partners for details on the copyright holders.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

function gen_arrange_script_gridsite()
{
remotehost=$1
COPYPROXY=$2

egrep -i "Debian|Ubuntu" /etc/issue
if [ \$? = 0 ]; then 
        INSTALLCMD="apt-get install -q --yes"
	INSTALLPKGS="lintian"
else
        INSTALLCMD="yum install -q -y --nogpgcheck"
	INSTALLPKGS="rpmlint"
fi

cat << EndArrangeScript > arrange_gridsite_test_root.sh 
CERTFILE=\$1
GLITE_USER=\$2
GSTSTCOLS=\$3
OUTPUT_OPT=\$4

echo "Certificate file: \$CERTFILE "
echo "gLite user:       \$GLITE_USER "
echo "Terminal width:   \$GSTSTCOLS "
echo "Output format:    \$OUTPUT_OPT "

export GSTSTCOLS

${INSTALLCMD} voms-clients httpd mod_ssl curl wget nc lsof $INSTALLPKGS

HTTPD_CONFDIR=/tmp
for dir in /etc/httpd /etc/apache /etc/apache2; do
	if [ -d \$dir ]; then
		HTTPD_CONFDIR=\$dir
		break
	fi
done
HTTPD_CONF=\$HTTPD_CONFDIR/gridsite-webserver.conf

sed -e '1,\$s!/usr/lib/httpd/modules/!modules/!' /usr/share/doc/gridsite-*/httpd-webserver.conf | sed 's!/var/www/html!/var/www/htdocs!' | sed "s/FULL.SERVER.NAME/\$(hostname -f)/" | sed "s/\(GridSiteGSIProxyLimit\)/# \1/"> \$HTTPD_CONF
echo "AddHandler cgi-script .cgi" >> \$HTTPD_CONF
echo "ScriptAlias /gridsite-delegation.cgi /usr/sbin/gridsite-delegation.cgi" >> \$HTTPD_CONF
mkdir -p /var/www/htdocs
killall httpd apache2 >/dev/null 2>&1
sleep 2
killall -9 httpd apache2 >/dev/null 2>&1
echo Starting httpd -f \$HTTPD_CONF
httpd -f \$HTTPD_CONF

cd /tmp

CVSPATH=\`which cvs\`

if [ "\$CVSPATH" = "" ]; then
        printf "CVS binary not present"
	${INSTALLCMD} cvs
fi

if [ $COPYPROXY -eq 1 ]; then
	mv \$CERTFILE x509up_u\`id -u\`
	chown \`id -un\`:\`id -gn\` x509up_u\`id -u\`
else
	rm -rf /tmp/test-certs/grid-security
	cvs -d :pserver:anonymous@glite.cvs.cern.ch:/cvs/jra1mw co org.glite.testsuites.ctb/LB > /dev/null 2>/dev/null
	./org.glite.testsuites.ctb/LB/tests/lb-generate-fake-proxy.sh > fake-prox.out.\$\$
	FAKE_CAS=\`cat fake-prox.out.\$\$ | grep -E "^X509_CERT_DIR" | sed 's/X509_CERT_DIR=//'\`
	if [ "\$FAKE_CAS" = "" ]; then
                echo "Failed generating proxy" >&2
                exit 2
        else
                cp -rv \$FAKE_CAS/* /etc/grid-security/certificates/
        fi

	TRUSTED_CERTS=\`cat fake-prox.out.\$\$ | grep -E "^TRUSTED_CERTS" | sed 's/TRUSTED_CERTS=//'\`
	export x509_USER_CERT=\${TRUSTED_CERTS}/trusted_client00.cert
	export x509_USER_KEY=\${TRUSTED_CERTS}/trusted_client00.priv-clear
	rm fake-prox.out.\$\$
fi

if [ ! -d /etc/vomses ]; then
	echo Installing experimental VOMS server
	if [ ! -f ./px-voms-install.sh ]; then
		wget -O px-voms-install.sh http://jra1mw.cvs.cern.ch/cgi-bin/jra1mw.cgi/org.glite.testsuites.ctb/PX/tests/px-voms-install.sh?view=co
		chmod +x px-voms-install.sh
	fi
	source ./px-voms-install.sh -u root
fi

cd ~/
mkdir GridSite_testing
cd GridSite_testing
cvs -d :pserver:anonymous@glite.cvs.cern.ch:/cvs/jra1mw co org.glite.testsuites.ctb/gridsite
cd org.glite.testsuites.ctb/gridsite/tests
echo ========================
echo "  REAL TESTS START HERE"
echo ========================
echo "</verbatim>"
echo "<literal>"
./ping-remote.sh $remotehost \$OUTPUT_OPT
./ping-local.sh \$OUTPUT_OPT -f \$HTTPD_CONF
./gridsite-test-all.sh \$OUTPUT_OPT
./gridsite-test-packaging.sh \$OUTPUT_OPT
echo "</literal>"
echo "<verbatim>"
echo ==================
echo "  TESTS END HERE"
echo ==================
echo "</verbatim>"

EndArrangeScript
}

