Summary: Information Supermarket for the Workload Management System
Name: glite-wms-ism
Version:
Release:
License: Apache Software License
Vendor: EMI
Packager: WMS group <wms-support@lists.infn.it>
URL: http://glite.cern.ch/
Group: System Environment/Libraries
BuildArch:
BuildRequires: %{!?extbuilddir: glite-wms-common-devel,} chrpath
BuildRequires: %{!?extbuilddir: glite-wms-utils-classad-devel,} libtool
BuildRequires: boost-devel, classads-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%global debug_package %{nil}

%description
Information Supermarket for the Workload Management System

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
sed 's|^prefix=.*|prefix=/usr|g' %{buildroot}%{_libdir}/pkgconfig/wms-ism.pc > %{buildroot}%{_libdir}/pkgconfig/wms-ism.pc.new
mv %{buildroot}%{_libdir}/pkgconfig/wms-ism.pc.new %{buildroot}%{_libdir}/pkgconfig/wms-ism.pc
sed 's|^prefix=.*|prefix=/usr|g' %{buildroot}%{_libdir}/pkgconfig/wms-ism-ii-purchaser.pc > %{buildroot}%{_libdir}/pkgconfig/wms-ism-ii-purchaser.pc.new
mv %{buildroot}%{_libdir}/pkgconfig/wms-ism-ii-purchaser.pc.new %{buildroot}%{_libdir}/pkgconfig/wms-ism-ii-purchaser.pc
rm -f %{buildroot}%{_libdir}/libglite_wms_ism*.la
chrpath --delete %{buildroot}%{_libdir}/libglite_wms_*.so.0.0.0
 

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%defattr(-,root,root)
%dir /usr/share/doc/glite-wms-ism-%{version}/
%doc /usr/share/doc/glite-wms-ism-%{version}/LICENSE
%{_libdir}/libglite_wms_ism*.so.0.0.0
%{_libdir}/libglite_wms_ism*.so.0

%changelog

%package devel
Summary: Development files for the WMS information superkmarket
Group: System Environment/Libraries
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: glite-wms-utils-classad-devel, glite-wms-common-devel
Requires: boost-devel, classads-devel

%description devel
Development files for the WMS information superkmarket

%files devel
%defattr(-,root,root)
%dir /usr/include/glite/
%dir /usr/include/glite/wms/
%dir /usr/include/glite/wms/ism/
%dir /usr/include/glite/wms/ism/purchaser/
/usr/include/glite/wms/ism/*.h
/usr/include/glite/wms/ism/purchaser/*.h
%{_libdir}/pkgconfig/wms-ism-ii-purchaser.pc
%{_libdir}/pkgconfig/wms-ism.pc
%{_libdir}/libglite_wms_*.so



