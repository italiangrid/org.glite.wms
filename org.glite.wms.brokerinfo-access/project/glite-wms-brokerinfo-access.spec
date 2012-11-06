Summary: Brokerinfo component for the WMS user interface
Name: glite-wms-brokerinfo-access
Version: %{extversion}
Release: %{extage}.%{extdist}
License: Apache Software License
Vendor: EMI
URL: http://glite.cern.ch/
Group: Applications/Internet
BuildArch: %{_arch}
BuildRequires: chrpath, libtool
BuildRequires: %{!?extbuilddir:glite-build-common-cpp, } classads-devel
BuildRequires: doxygen, docbook-style-xsl, libxslt
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%global debug_package %{nil}

%description
Brokerinfo component for the WMS user interface

%prep
 

%setup -c -q

%build
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  ./configure --prefix=%{buildroot}/usr --disable-static PVER=%{version}
  make
  make doxygen-doc
fi


%install
rm -rf %{buildroot}
mkdir -p %{buildroot}
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  make install
  mkdir -p %{buildroot}/%{_docdir}/%{name}
  cp -R autodoc/html %{buildroot}/%{_docdir}/%{name}
else
  cp -R %{extbuilddir}/* %{buildroot}
fi
sed 's|^prefix=.*|prefix=/usr|g' %{buildroot}/usr/lib64/pkgconfig/brokerinfo-access.pc > %{buildroot}/usr/lib64/pkgconfig/brokerinfo-access.pc.new
mv %{buildroot}/usr/lib64/pkgconfig/brokerinfo-access.pc.new %{buildroot}/usr/lib64/pkgconfig/brokerinfo-access.pc
rm %{buildroot}/usr/lib64/*.la
chrpath --delete %{buildroot}/usr/lib64/libglite-brokerinfo.so.0.0.0
chrpath --delete %{buildroot}/usr/bin/glite-brokerinfo
export QA_SKIP_BUILD_ROOT=yes

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
/usr/bin/glite-brokerinfo
%doc /usr/share/man/man1/glite-brokerinfo.1.gz



%package lib
Summary: Brokerinfo component for the WMS user interface (libraries)
Group: System Environment/Libraries

%description lib
Brokerinfo component for the WMS user interface (libraries)

%files lib
%defattr(-,root,root)
/usr/lib64/libglite-brokerinfo.so.0
/usr/lib64/libglite-brokerinfo.so.0.0.0
%dir /usr/share/doc/glite-wms-brokerinfo-access-%{version}/
%doc /usr/share/doc/glite-wms-brokerinfo-access-%{version}/LICENSE

%post lib -p /sbin/ldconfig

%postun lib -p /sbin/ldconfig


%package devel
Summary: Brokerinfo component for the WMS user interface (development files)
Group: System Environment/Libraries
Requires: %{name}-lib%{?_isa} = %{version}-%{release}
Requires: classads-devel, glite-build-common-cpp

%description devel
Brokerinfo component for the WMS user interface (development files)

%files devel
%defattr(-,root,root)
%dir /usr/include/glite/
%dir /usr/include/glite/wms/
%dir /usr/include/glite/wms/brokerinfo-access/
/usr/include/glite/wms/brokerinfo-access/*.h
/usr/lib64/pkgconfig/brokerinfo-access.pc
/usr/lib64/libglite-brokerinfo.so

%package doc
Summary: Documentation files for the brokerinfo access component
Group: Documentation

%description doc
Documentation files for the brokerinfo access component

%files doc
%defattr(-,root,root)
%dir %{_docdir}/%{name}/html
%doc %{_docdir}/%{name}/html/*.html
%doc %{_docdir}/%{name}/html/*.css
%doc %{_docdir}/%{name}/html/*.png
%doc %{_docdir}/%{name}/html/*.gif



%changelog
* %{extcdate} WMS group <wms-support@lists.infn.it> - %{extversion}-%{extage}.%{extdist}
- %{extclog}


