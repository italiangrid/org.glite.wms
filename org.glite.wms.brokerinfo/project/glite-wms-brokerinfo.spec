Summary:"files for gLite wms brokerinfo"
Name:glite-wms-brokerinfo
Version:2.0.0
Release:1
Copyright:Open Source EGEE License
Vendor:EU EGEE project
Group:System/Application
Prefix:/opt/glite
BuildArch:i386
Requires: glite-wms-common glite-wms-jdl glite-wms-rls glite-wms-ism
BuildRoot:%{_builddir}/%{name}-%{version}
Source:glite-wms-brokerinfo-2.0.0_bin.tar.gz

%define debug_package %{nil}
%define    __spec_install_post     %{nil}

%description
"brokerinfo"

%prep
 

%setup -c

%build
 

%install
 

%clean
 
%pre
%post
# ----- inserted from file fix-libtool-path.sh
SED="s|/home/ibrido/release15/stage|${RPM_INSTALL_PREFIX}|g"
for la in ${RPM_INSTALL_PREFIX}/lib/libglite_wms_brokerinfo.la ${RPM_INSTALL_PREFIX}/lib/libglite_wms_rls.la ${RPM_INSTALL_PREFIX}/lib/libglite_wms_dli_sici.la  
do
	sed -e $SED ${la} > ${la}.new && \
	mv -f ${la}.new ${la}
done
%preun
%postun
%files
%defattr(-,root,root)
%{prefix}/include/glite/wms/brokerinfo/BrokerInfoData.h
%{prefix}/include/glite/wms/brokerinfo/brokerinfoGlueImpl.h
%{prefix}/include/glite/wms/brokerinfo/brokerinfo.h
%{prefix}/include/glite/wms/brokerinfo/glue_attributes.h
%{prefix}/lib/libglite_wms_brokerinfo.la
%{prefix}/lib/libglite_wms_brokerinfo.a
%{prefix}/lib/libglite_wms_rls.so.0.0.0
%{prefix}/lib/libglite_wms_rls.so.0
%{prefix}/lib/libglite_wms_rls.so
%{prefix}/lib/libglite_wms_rls.la
%{prefix}/lib/libglite_wms_rls.a
%{prefix}/lib/libglite_wms_dli_sici.so.0.0.0
%{prefix}/lib/libglite_wms_dli_sici.so.0
%{prefix}/lib/libglite_wms_dli_sici.so
%{prefix}/lib/libglite_wms_dli_sici.la
%{prefix}/lib/libglite_wms_dli_sici.a
%{prefix}/share/doc/glite-wms-brokerinfo-2.0.0/LICENSE

%changelog

