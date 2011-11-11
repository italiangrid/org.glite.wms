Summary: Workload Management Proxy service
Name: glite-wms-wmproxy
Version:
Release:
License: Apache Software License
Vendor: EMI
Packager: WMS group <wms-support@lists.infn.it>
URL: http://glite.cern.ch/
Group: Applications/Internet
BuildArch:
Requires: mod_fcgid
Requires: httpd
Requires: mod_ssl
Requires: gridsite-apache
Requires: gridsite-shared
Requires: glite-wms-configuration
Requires: lcmaps-plugins-basic
Requires(post): chkconfig
Requires(preun): chkconfig
Requires(preun): initscripts
BuildRequires: %{!?extbuilddir: gridsite-devel, glite-jobid-api-cpp, voms-devel,} chrpath, boost-devel
BuildRequires: %{!?extbuilddir: argus-pep-api-c-devel, glite-wms-purger-devel,} libtool
BuildRequires: %{!?extbuilddir: lcmaps-without-gsi, lcmaps-interface,} classads-devel
BuildRequires: %{!?extbuilddir: glite-jdl-api-cpp-devel, glite-lb-client,} fcgi-devel
BuildRequires: %{!?extbuilddir: glite-px-proxyrenewal,} libxslt-devel
BuildRequires: %{!?extbuilddir: glite-wms-wmproxy-interface,} libtar-devel
BuildRequires: gsoap-devel, httpd-devel, zlib-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%global debug_package %{nil}

%description
Workload Management Proxy service

%prep
 

%setup -c -q

%build
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  ./configure --srcdir=$PWD --prefix=%{buildroot}/usr --sysconfdir=%{buildroot}/etc --disable-static PVER=%{version}
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
rm %{buildroot}%{_libdir}/*.la
chrpath --delete %{buildroot}%{_libdir}/*.so.0.0.0
chrpath --delete %{buildroot}/usr/bin/glite_wms_wmproxy_server

%clean
rm -rf %{buildroot}

%post
/sbin/ldconfig
/sbin/chkconfig --add glite-wms-wmproxy
chown root.root /usr/libexec/glite_wms_wmproxy_dirmanager
chmod u+s /usr/libexec/glite_wms_wmproxy_dirmanager

%preun
if [ $1 -eq 0 ] ; then
    /sbin/service glite-wms-wmproxy stop >/dev/null 2>&1
    /sbin/chkconfig --del glite-wms-wmproxy
fi

%postun -p /sbin/ldconfig




%files
%defattr(-,root,root)
%dir /etc/glite-wms/
%dir /etc/lcmaps/
%config(noreplace) /etc/glite-wms/wmproxy_httpd.conf.template
%config(noreplace) /etc/glite-wms/wmproxy.gacl.template
%config(noreplace) /etc/glite-wms/wmproxy_logrotate.conf.template
%config(noreplace) /etc/lcmaps/lcmaps.db.template
/etc/rc.d/init.d/glite-wms-wmproxy
/usr/sbin/glite_wms_wmproxy_load_monitor
/usr/bin/glite_wms_wmproxy_server
/usr/bin/glite-wms-wmproxy-purge-proxycache
#/usr/bin/glite-wms-wmproxy-gacladmin
#/usr/bin/glite-wms-wmproxy-purge-proxycache-binary
#/usr/bin/glite-wms-wmproxy-gridmapfile2gacl
%dir /usr/share/doc/glite-wms-wmproxy-%{version}/
%doc /usr/share/doc/glite-wms-wmproxy-%{version}/LICENSE
%{_libdir}/libglite_wms_wmproxy_*.so.0.0.0
%{_libdir}/libglite_wms_wmproxy_*.so.0
%{_libdir}/libglite_wms_wmproxy_*.so
/usr/libexec/glite_wms_wmproxy_dirmanager

%changelog

 
