Summary: Command line user interface for the WMS
Name: glite-wms-ui-commands
Version:
Release:
License: Apache Software License
Vendor: EMI
Packager: WMS group <wms-support@lists.infn.it>
URL: http://glite.cern.ch/
Group: System Environment/Libraries
BuildArch: x86_64
#Requires: glite-wms-wmproxy-api-cpp
#Requires: glite-jobid-api-cpp
#Requires: libtar
#Requires: classads
Requires: glite-wms-ui-api-python
#Requires: glite-service-discovery-api-c
#Requires: zlib
#Requires: glite-lb-client
#Requires: globus-gass-copy-progs
#Requires: gridsite-shared
#Requires: glite-jdl-api-cpp
#Requires: boost
#Requires: glite-wms-utils-exception
BuildRequires: chrpath
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%description
Command line user interface for the WMS

%prep
 

%setup -c

%build
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  ./configure --prefix=%{buildroot}/usr --sysconfdir=%{buildroot}/etc --with-gsoap-version=2.7.13 --disable-static PVER=%{version}
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
chrpath --delete %{buildroot}/usr/bin/glite-wms-job-cancel
chrpath --delete %{buildroot}/usr/bin/glite-wms-job-delegate-proxy
chrpath --delete %{buildroot}/usr/bin/glite-wms-job-info
chrpath --delete %{buildroot}/usr/bin/glite-wms-job-list-match
chrpath --delete %{buildroot}/usr/bin/glite-wms-job-output
chrpath --delete %{buildroot}/usr/bin/glite-wms-job-perusal
chrpath --delete %{buildroot}/usr/bin/glite-wms-job-submit

%clean
rm -rf %{buildroot}
 

%files
%defattr(-,root,root)
%config(noreplace) /etc/glite_wmsui_cmd_*
/usr/bin/glite-wms-job-*
%dir /usr/share/doc/glite-wms-ui-commands-%{version}/
%doc /usr/share/doc/glite-wms-ui-commands-%{version}/LICENSE
%doc /usr/share/man/man1/*.1.gz

%changelog
 
