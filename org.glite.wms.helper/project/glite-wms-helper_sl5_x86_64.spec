Summary: Helper module for the Workload Management System
Name: glite-wms-helper
Version: %{extversion}
Release: %{extage}.%{extdist}
License: Apache Software License
Vendor: EMI
URL: http://glite.cern.ch/
Group: System Environment/Libraries
BuildArch: %{_arch}
BuildRequires: %{!?extbuilddir: glite-wms-broker-devel,} chrpath
BuildRequires: %{!?extbuilddir: glite-jobid-api-cpp-devel,} libtool
BuildRequires: %{!?extbuilddir: glite-wms-matchmaking-devel,} boost-devel
BuildRequires: classads-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%global debug_package %{nil}

%description
Helper module for the Workload Management System

%prep
 

%setup -c -q

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
sed 's|^prefix=.*|prefix=/usr|g' %{buildroot}%{_libdir}/pkgconfig/wms-helper.pc > %{buildroot}%{_libdir}/pkgconfig/wms-helper.pc.new
mv %{buildroot}%{_libdir}/pkgconfig/wms-helper.pc.new %{buildroot}%{_libdir}/pkgconfig/wms-helper.pc
sed 's|^prefix=.*|prefix=/usr|g' %{buildroot}%{_libdir}/pkgconfig/wms-helper-jobadapter.pc > %{buildroot}%{_libdir}/pkgconfig/wms-helper-jobadapter.pc.new
mv %{buildroot}%{_libdir}/pkgconfig/wms-helper-jobadapter.pc.new %{buildroot}%{_libdir}/pkgconfig/wms-helper-jobadapter.pc
sed 's|^prefix=.*|prefix=/usr|g' %{buildroot}%{_libdir}/pkgconfig/wms-helper-broker-ism.pc > %{buildroot}%{_libdir}/pkgconfig/wms-helper-broker-ism.pc.new
mv %{buildroot}%{_libdir}/pkgconfig/wms-helper-broker-ism.pc.new %{buildroot}%{_libdir}/pkgconfig/wms-helper-broker-ism.pc
rm %{buildroot}%{_libdir}/*.la
chrpath --delete %{buildroot}%{_libdir}/libglite_wms_helper_*.so.0.0.0


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
%{_libdir}/libglite_wms_helper*.so.0.0.0
%{_libdir}/libglite_wms_helper*.so.0




%package devel
Summary: Development files for the WMS helper module
Group: System Environment/Libraries
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: glite-wms-broker-devel
Requires: glite-jobid-api-cpp-devel
Requires: glite-wms-matchmaking-devel


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
%{_libdir}/pkgconfig/wms-helper-jobadapter.pc
%{_libdir}/pkgconfig/wms-helper-broker-ism.pc
%{_libdir}/pkgconfig/wms-helper.pc
%{_libdir}/libglite_wms_helper*.so


%changelog
* %(date +"%%a %%b %%d %%Y") WMS group <wms-support@lists.infn.it> - %{version}-%{release}
- %{extclog}

