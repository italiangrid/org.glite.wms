Summary: Python libraries for the WM Proxy service
Name: glite-wms-wmproxy-api-python
Version:
Release:
License: Apache Software License
Vendor: EMI
Packager: WMS group <wms-support@lists.infn.it>
URL: http://glite.cern.ch/
Group: System Environment/Libraries
BuildArch: noarch
Requires: python-fpconst
Requires: PyXML
Requires: SOAPpy
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%description
Python libraries for the WM Proxy service

%prep
 

%setup -c

%build
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  ./configure --prefix=%{buildroot}/usr PVER=%{version}
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

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%dir /usr/share/doc/glite-wms-wmproxy-api-python-%{version}/
%doc /usr/share/doc/glite-wms-wmproxy-api-python-%{version}/LICENSE
%{python_sitearch}/*.py
%{python_sitearch}/*.pyc
%{python_sitearch}/*.pyo

%changelog
 
