Summary: Helper module for the Workload Management System
Name: glite-wms-helper
Version:
Release:
License: Apache Software License
Vendor: EMI
Packager: WMS group <wms-support@lists.infn.it>
URL: http://glite.cern.ch/
Group: System Environment/Libraries
BuildArch: x86_64
#Requires: glite-wms-broker
#Requires: glite-wms-common
#Requires: glite-jobid-api-c
#Requires: glite-wms-brokerinfo
#Requires: classads
#Requires: glite-wms-classad_plugin
#Requires: glite-jdl-api-cpp
#Requires: glite-wms-matchmaking
#Requires: glite-wms-utils-classad
#Requires: boost
#Requires: glite-wms-utils-exception
BuildRequires: chrpath
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%description
Helper module for the Workload Management System

%prep
 

%setup -c

%build
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  ./configure --prefix=%{buildroot}/usr --disable-static PVER=%{version}
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
sed 's|^prefix=.*|prefix=/usr|g' %{buildroot}/usr/lib64/pkgconfig/wms-helper.pc > %{buildroot}/usr/lib64/pkgconfig/wms-helper.pc.new
mv %{buildroot}/usr/lib64/pkgconfig/wms-helper.pc.new %{buildroot}/usr/lib64/pkgconfig/wms-helper.pc
sed 's|^prefix=.*|prefix=/usr|g' %{buildroot}/usr/lib64/pkgconfig/wms-helper-jobadapter.pc > %{buildroot}/usr/lib64/pkgconfig/wms-helper-jobadapter.pc.new
mv %{buildroot}/usr/lib64/pkgconfig/wms-helper-jobadapter.pc.new %{buildroot}/usr/lib64/pkgconfig/wms-helper-jobadapter.pc
sed 's|^prefix=.*|prefix=/usr|g' %{buildroot}/usr/lib64/pkgconfig/wms-helper-broker-ism.pc > %{buildroot}/usr/lib64/pkgconfig/wms-helper-broker-ism.pc.new
mv %{buildroot}/usr/lib64/pkgconfig/wms-helper-broker-ism.pc.new %{buildroot}/usr/lib64/pkgconfig/wms-helper-broker-ism.pc
rm %{buildroot}/usr/lib64/*.la
chrpath --delete %{buildroot}/usr/lib64/libglite_wms_helper_*.so.0.0.0


%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig
 

%files
%defattr(-,root,root)
%dir /usr/share/glite-wms/
/usr/share/glite-wms/jobwrapper.template.sh
%dir /usr/share/doc/glite-wms-helper-%{version}/
/usr/share/doc/glite-wms-helper-%{version}/LICENSE
/usr/lib64/libglite_wms_helper_*.so.0.0.0
/usr/lib64/libglite_wms_helper_*.so.0

%changelog



%package devel
Summary: Development files for the WMS helper module
Group: System Environment/Libraries

%description devel
Development files for the WMS helper module

%files devel
%defattr(-,root,root)
%dir /usr/include/glite/
%dir /usr/include/glite/wms/
%dir /usr/include/glite/wms/helper/
%dir /usr/include/glite/wms/helper/jobadapter/
/usr/include/glite/wms/helper/*.h
/usr/include/glite/wms/helper/jobadapter/*.h
/usr/lib64/pkgconfig/wms-helper-jobadapter.pc
/usr/lib64/pkgconfig/wms-helper-broker-ism.pc
/usr/lib64/pkgconfig/wms-helper.pc
/usr/lib64/libglite_wms_helper_*.so

