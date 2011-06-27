Summary: Workload Management Proxy service
Name: glite-wms-wmproxy
Version:
Release:
License: Apache Software License
Vendor: EMI
Packager: WMS group <wms-support@lists.infn.it>
URL: http://glite.cern.ch/
Group: Applications/Internet
BuildArch: x86_64
#Requires: glite-jobid-api-cpp
#Requires: fcgi
#Requires: libxslt
Requires: mod_fcgid
Requires: httpd
#Requires: glite-wms-common
Requires: mod_ssl
#Requires: glite-jdl-api-cpp
Requires: gridsite-apache
#Requires: boost
#Requires: glite-service-discovery-api-c
#Requires: argus-pep-api-c
#Requires: glite-px-proxyrenewal
#Requires: gsoap
#Requires: voms
#Requires: lcmaps-without-gsi
Requires: gridsite-shared
#Requires: libtar
#Requires: classads
#Requires: zlib
#Requires: glite-lb-client
#Requires: glite-wms-utils-classad
#Requires: glite-wms-configuration
#Requires: glite-wms-purger
Requires: lcmaps-plugins-basic
BuildRequires: chrpath
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%description
Workload Management Proxy service

%prep
 

%setup -c

%build
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  ./configure --prefix=%{buildroot}/usr --sysconfdir=%{buildroot}/etc --with-gsoap-version=2.7.13 --disable-static PVER=%{version}
  make
fi

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  make install
else
  cp -R %{extbuilddir}/* %{buildroot}
fi
rm %{buildroot}/usr/lib64/*.la
chrpath --delete %{buildroot}/usr/lib64/*.so.0.0.0


%clean
rm -rf %{buildroot}

%post
/sbin/ldconfig
chown root.root /usr/libexec/glite_wms_wmproxy_dirmanager
chmod u+s /usr/libexec/glite_wms_wmproxy_dirmanager

%postun -p /sbin/ldconfig




%files
%defattr(-,root,root)
%dir /etc/glite-wms/
%dir /etc/lcmaps/
%config(noreplace) /etc/glite-wms/wmproxy_httpd.conf.template
%config(noreplace) /etc/glite-wms/wmproxy.gacl.template
%config(noreplace) /etc/glite-wms/wmproxy_logrotate.conf.template
%config(noreplace) /etc/lcmaps/lcmaps.db.template
%dir /etc/rc.d/init.d/
/etc/rc.d/init.d/glite-wms-wmproxy
/usr/sbin/glite_wms_wmproxy_load_monitor
/usr/bin/glite_wms_wmproxy_server
/usr/bin/glite-wms-wmproxy-purge-proxycache
/usr/bin/glite-wms-wmproxy-gacladmin
/usr/bin/glite-wms-wmproxy-purge-proxycache-binary
/usr/bin/glite-wms-wmproxy-gridmapfile2gacl
%dir /usr/share/doc/glite-wms-wmproxy-%{version}/
%doc /usr/share/doc/glite-wms-wmproxy-%{version}/LICENSE
/usr/lib64/libglite_wms_wmproxy_*.so.0.0.0
/usr/lib64/libglite_wms_wmproxy_*.so.0
/usr/lib64/libglite_wms_wmproxy_*.so
%dir /usr/libexec/
/usr/libexec/glite_wms_wmproxy_dirmanager

%changelog
 
