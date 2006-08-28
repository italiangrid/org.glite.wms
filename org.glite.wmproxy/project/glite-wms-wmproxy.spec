Summary:Files for gLite WMS WMProxy Service
Name:glite-wms-wmproxy
Version:@MODULE.VERSION@
Release:@MODULE.BUILD@
Copyright:Open Source EGEE License
Vendor:EU EGEE project
Group:System/Application
Prefix:/opt/glite
BuildArch:i386
Requires: mod_ssl gridsite_module glite-wms-partitioner glite-jdl-api-cpp glite-wms-common glite-wms-checkpointing glite-wms-purger glite-wms-configuration
BuildRoot:%{_builddir}/%{name}-%{version}
Source:%{name}-%{version}_bin.tar.gz

%define debug_package %{nil}

%description
gLite WMS WMProxy Service executables and libraries

%prep
 

%setup -c

%build
 

%install
 

%clean
 
%pre
%post
%preun
%postun

%files
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_utilities.so.0.0.0
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_utilities.so.0
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_utilities.so
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_utilities.a
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_eventlogger.so.0.0.0
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_eventlogger.so.0
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_eventlogger.so
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_eventlogger.a
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_authorizer.so.0.0.0
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_authorizer.so.0
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_authorizer.so
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_authorizer.a
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_commands.so.0.0.0
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_commands.so.0
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_commands.so
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_commands.a
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_pipe.so.0.0.0
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_pipe.so.0
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_pipe.so
%attr(-, root, root) %{prefix}/lib/libglite_wms_wmproxy_pipe.a
#%attr(-, root, root) %{prefix}/bin/glite_wms_wmproxy_purge_proxycache
%attr(4555, root, root) %{prefix}/bin/glite_wms_wmproxy_dirmanager
%attr(-, root, root) %{prefix}/bin/glite_wms_wmproxy_server
%attr(-, root, root) %{prefix}/sbin/glite_wms_wmproxy_load_monitor.template
%attr(-, root, root) %{prefix}/bin/glite-wms-wmproxy-gacladmin
%attr(-, root, root) %{prefix}/bin/glite-wms-wmproxy-gridmapfile2gacl
%attr(-, root, root) %{prefix}/bin/glite-wms-wmproxy-purge-proxycache
%attr(-, root, root) %{prefix}/etc/lcmaps/lcmaps.db.template
%attr(-, root, root) %{prefix}/etc/glite_wms_wmproxy_httpd.conf.template
%attr(-, root, root) %{prefix}/etc/glite_wms_wmproxy.gacl.template
%attr(-, root, root) %{prefix}/etc/init.d/glite-wms-wmproxy
%attr(-, root, root) %{prefix}/share/doc/glite-wms-wmproxy-%{version}/LICENSE
%attr(-, root, root) %{prefix}/share/doc/glite-wms-wmproxy-%{version}/CHANGES

%changelog

