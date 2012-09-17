#!/bin/bash

# Check_vomscerts.sh -- shell script to sanity check VOMSCERT rpm
# Andrew Elwell <andrew.elwell@cern.ch>
# Jan 2009
#
# $Id$
#

# Show Arch too
QUERYFMT='%{N}-%{V}.%{ARCH}'
RPM=lcg-vomscerts

rpm -q --qf $QUERYFMT $RPM
for pem in `rpm -ql $RPM | grep pem$` ; do
	cert=`basename $pem .pem`
	echo -e "\n>>>> $cert <<<<"
	openssl x509  -in $pem -noout -dates -subject -issuer -serial -fingerprint
done
