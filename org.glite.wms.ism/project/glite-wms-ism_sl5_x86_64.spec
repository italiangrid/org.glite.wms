Summary: Information Supermarket for the Workload Management System
Name: glite-wms-ism
Version:
Release:
License: Apache Software License
Vendor: EMI
Packager: WMS group <wms-support@lists.infn.it>
URL: http://glite.cern.ch/
Group: System Environment/Libraries
BuildArch: x86_64
#Requires: glite-wms-common
#Requires: boost
#Requires: classads
#Requires: glite-wms-utils-classad
BuildRequires: chrpath
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%description
Information Supermarket for the Workload Management System

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
sed 's|^prefix=.*|prefix=/usr|g' %{buildroot}/usr/lib64/pkgconfig/wms-ism.pc > %{buildroot}/usr/lib64/pkgconfig/wms-ism.pc.new
mv %{buildroot}/usr/lib64/pkgconfig/wms-ism.pc.new %{buildroot}/usr/lib64/pkgconfig/wms-ism.pc
sed 's|^prefix=.*|prefix=/usr|g' %{buildroot}/usr/lib64/pkgconfig/wms-ism-ii-purchaser.pc > %{buildroot}/usr/lib64/pkgconfig/wms-ism-ii-purchaser.pc.new
mv %{buildroot}/usr/lib64/pkgconfig/wms-ism-ii-purchaser.pc.new %{buildroot}/usr/lib64/pkgconfig/wms-ism-ii-purchaser.pc
rm -f /usr/lib64/libglite_wms_ism*.la
chrpath --delete %{buildroot}/usr/lib64/libglite_wms_*.so.0.0.0
 

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%defattr(-,root,root)
%dir /usr/share/doc/glite-wms-ism-%{version}/
%doc /usr/share/doc/glite-wms-ism-%{version}/LICENSE
/usr/lib64/libglite_wms_ism*.so.0.0.0
/usr/lib64/libglite_wms_ism*.so.0

%changelog

%package devel
Summary: Development files for the WMS information superkmarket
Group: System Environment/Libraries

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
/usr/lib64/pkgconfig/wms-ism-ii-purchaser.pc
/usr/lib64/pkgconfig/wms-ism.pc
/usr/lib64/libglite_wms_*.so

