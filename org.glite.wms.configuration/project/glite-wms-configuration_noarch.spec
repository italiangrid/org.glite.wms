Summary: Configuration module for the Workload Management System
Name: glite-wms-configuration
Version:
Release:
License: Apache Software License
Vendor: EMI
URL: http://glite.cern.ch/
Packager: WMS group <wms-support@lists.infn.it>
Group: System Environment/Libraries
BuildArch: noarch
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%description
Configuration module for the Workload Management System

%prep
 

%setup -c

%build
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  ./configure --prefix=%{buildroot}/usr --sysconfdir=%{buildroot}/etc PVER=%{version}
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
%dir /etc/glite-wms/
%config(noreplace) /etc/glite-wms/wms.conf.template
%dir /usr/share/doc/glite-wms-configuration-%{version}/
%doc /usr/share/doc/glite-wms-configuration-%{version}/LICENSE
/usr/libexec/glite-wms-parse-configuration.sh
/usr/libexec/glite-wms-check-daemons.sh
/usr/libexec/glite-wms-services-certs.sh

%changelog


