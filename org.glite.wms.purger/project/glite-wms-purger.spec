Summary: Cleanup module for the Workload Management System
Name: glite-wms-purger
Version: %{extversion}
Release: %{extage}.%{extdist}
License: ASL 2.0
Vendor: EMI
URL: http://glite.cern.ch/
Group: System Environment/Libraries
BuildArch: %{_arch}
BuildRequires: %{!?extbuilddir: glite-wms-common-devel,} chrpath
BuildRequires: %{!?extbuilddir: glite-jobid-api-cpp-devel, glite-lb-client-devel, } libtool
BuildRequires: %{!?extbuilddir: glite-wms-utils-classad-devel,} boost-devel
BuildRequires: %{!?extbuilddir: glite-px-proxyrenewal-devel,} classads-devel
BuildRequires: %{!?extbuilddir: glite-build-common-cpp, } globus-gss-assist-devel
BuildRequires: %{!?extbuilddir: glite-jobid-api-c-devel, } c-ares-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%global debug_package %{nil}

%description
Cleanup module for the Workload Management System

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
sed 's|^prefix=.*|prefix=/usr|g' %{buildroot}%{_libdir}/pkgconfig/wms-purger.pc > %{buildroot}%{_libdir}/pkgconfig/wms-purger.pc.new
mv %{buildroot}%{_libdir}/pkgconfig/wms-purger.pc.new %{buildroot}%{_libdir}/pkgconfig/wms-purger.pc
rm %{buildroot}%{_libdir}/*.la
chrpath --delete %{buildroot}%{_libdir}/libglite_wms_*.so.0.0.0
chrpath --delete %{buildroot}/usr/sbin/glite-wms-purgeStorage 
export QA_SKIP_BUILD_ROOT=yes

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%defattr(-,root,root)
/usr/sbin/glite-wms-purgeStorage
/usr/sbin/glite-wms-create-proxy.sh
/usr/sbin/glite-wms-purgeStorage.sh
%dir /usr/share/doc/glite-wms-purger-%{version}/
%doc /usr/share/doc/glite-wms-purger-%{version}/LICENSE
%{_libdir}/libglite_wms_purger.so.0
%{_libdir}/libglite_wms_purger.so.0.0.0



%package devel
Summary: Development files for the WMS purger module
Group: System Environment/Libraries
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: glite-wms-common-devel, glite-jobid-api-cpp-devel
Requires: glite-lb-client-devel, glite-wms-utils-classad-devel
Requires: glite-px-proxyrenewal-devel, boost-devel
Requires: c-ares-devel, globus-gss-assist-devel
Requires: glite-build-common-cpp, glite-jobid-api-c-devel

%description devel
Development files for the WMS purger module

%files devel
%defattr(-,root,root)
%dir /usr/include/glite/
%dir /usr/include/glite/wms/
%dir /usr/include/glite/wms/purger/
/usr/include/glite/wms/purger/purger.h
%{_libdir}/pkgconfig/wms-purger.pc
%{_libdir}/libglite_wms_purger.so



%changelog
* %{extcdate} WMS group <wms-support@lists.infn.it> - %{extversion}-%{extage}.%{extdist}
- %{extclog}

