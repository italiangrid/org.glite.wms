#!/bin/sh

#bug #32360: glite-sd2cache error

cat > file <<EOF
<services> 
<service name="myproxy.cern.ch:7512">
<parameters>
<endpoint>myproxy.cern.ch:7512</endpoint>
<type>MyProxy</type>
<version>1.1.0</version>
</parameters>
</service>

<service name="myproxy.cern.ch:7512">
<parameters>
<endpoint>myproxy.cern.ch:7512</endpoint>
<type>MyProxy</type>
<version>1.1.0</version>
</parameters>
</service>
</services> 
EOF

xsltproc /opt/glite/var/lib/sd2cache/glite-sd2cache-proxyfix.xslt file

if [ $? -ne 0 ]; then
  echo "BUG PRESENT"
  exit 1
else
  echo "BUG FIXED"
  exit 0
fi

