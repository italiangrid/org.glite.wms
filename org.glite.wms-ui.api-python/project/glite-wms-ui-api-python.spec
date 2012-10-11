Summary: Python libraries for WMS user interface
Name: glite-wms-ui-api-python
Version: %{extversion}
Release: %{extage}.%{extdist}
License: Apache Software License
Vendor: EMI
URL: http://glite.cern.ch/
Group: System Environment/Libraries
BuildArch: %{_arch}
BuildRequires: %{!?extbuilddir: glite-jdl-api-cpp-devel, glite-jobid-api-cpp-devel,} swig
BuildRequires: %{!?extbuilddir: glite-lb-client-devel, voms-devel, gridsite-devel,} classads-devel
BuildRequires: %{!?extbuilddir: glite-wms-utils-exception-devel,} boost-devel, python-devel
BuildRequires: %{!?extbuilddir:glite-build-common-cpp, } chrpath, libtool
BuildRequires: cppunit-devel, libxml2-devel
Requires: swig
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%global debug_package %{nil}

%if ! (0%{?fedora} > 12 || 0%{?rhel} > 5)
%{!?python_sitelib: %global python_sitelib %(%{__python} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())")}
%{!?python_sitearch: %global python_sitearch %(%{__python} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(1))")}
%endif

%description
Python libraries for WMS user interface

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
rm %{buildroot}/usr/lib64/*.la %{buildroot}/usr/lib64/*.a
chrpath --delete %{buildroot}/usr/lib64/*.so.0.0.0

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
/usr/lib64/*.so.0.0.0
/usr/lib64/*.so.0
/usr/lib64/*.so

/usr/lib64/python/glite_wmsui_AdWrapper.py
/usr/lib64/python/glite_wmsui_AdWrapper.pyc
/usr/lib64/python/glite_wmsui_LbWrapper.py
/usr/lib64/python/glite_wmsui_LbWrapper.pyc
/usr/lib64/python/glite_wmsui_LogWrapper.py
/usr/lib64/python/glite_wmsui_LogWrapper.pyc
/usr/lib64/python/glite_wmsui_UcWrapper.py
/usr/lib64/python/glite_wmsui_UcWrapper.pyc
/usr/lib64/python/wmsui_api.py
/usr/lib64/python/wmsui_api.pyc
/usr/lib64/python/wmsui_checks.py
/usr/lib64/python/wmsui_checks.pyc
/usr/lib64/python/wmsui_listener.py
/usr/lib64/python/wmsui_listener.pyc
/usr/lib64/python/wmsui_utils.py
/usr/lib64/python/wmsui_utils.pyc

#%{python_sitearch}/*.py
#%{python_sitearch}/*.pyc
#%{python_sitearch}/*.pyo
#%dir /usr/share/doc/%{name}-%{version}/
#%doc /usr/share/doc/%{name}-%{version}/LICENSE

%changelog
* %{extcdate} WMS group <wms-support@lists.infn.it> - %{extversion}-%{extage}.%{extdist}
- %{extclog}
