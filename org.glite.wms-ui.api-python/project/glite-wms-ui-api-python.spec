Summary: Python libraries for WMS user interface
Name: glite-wms-ui-api-python
Version: %{extversion}
Release: %{extage}.%{extdist}
License: ASL 2.0
Vendor: EMI
URL: http://glite.cern.ch/
Group: System Environment/Libraries
BuildArch: %{_arch}
BuildRequires: %{!?extbuilddir: glite-jdl-api-cpp-devel, glite-jobid-api-cpp-devel,} swig
BuildRequires: %{!?extbuilddir: glite-lb-client-devel, voms-devel, gridsite-devel,} classads-devel
BuildRequires: %{!?extbuilddir: glite-wms-utils-exception-devel,} boost-devel, python-devel
BuildRequires: %{!?extbuilddir:glite-build-common-cpp, } chrpath, cmake, globus-gss-assist-devel
BuildRequires: cppunit-devel, libxml2-devel, emi-pkgconfig-compat
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
%{!?extbuilddir:%define extbuilddir "-"}
%setup -c -q

%build
if test "x%{extbuilddir}" == "x-" ; then
  cmake -DPREFIX:string=%{buildroot}/usr -DPVER:string=%{version} .
fi

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x-" ; then
  make install
else
  cp -R %{extbuilddir}/* %{buildroot}
fi
chrpath --delete %{buildroot}/usr/lib64/*.so.0.0.0
strip -s %{buildroot}/usr/lib64/*.so.0.0.0

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
/usr/lib64/*.so.0.0.0
/usr/lib64/*.so.0
/usr/lib64/*.so

/usr/lib64/python/glite_wmsui_AdWrapper.*
/usr/lib64/python/glite_wmsui_SdWrapper.*
/usr/lib64/python/glite_wmsui_LbWrapper.*
/usr/lib64/python/glite_wmsui_LogWrapper.*
/usr/lib64/python/glite_wmsui_UcWrapper.*
/usr/lib64/python/wmsui_api.*
/usr/lib64/python/wmsui_checks.*
/usr/lib64/python/wmsui_listener.*
/usr/lib64/python/wmsui_utils.*

#%{python_sitearch}/*.py
#%{python_sitearch}/*.pyc
#%{python_sitearch}/*.pyo
#%dir /usr/share/doc/%{name}-%{version}/
#%doc /usr/share/doc/%{name}-%{version}/LICENSE

%changelog
* %{extcdate} WMS group <wms-support@lists.infn.it> - %{extversion}-%{extage}.%{extdist}
- %{extclog}
