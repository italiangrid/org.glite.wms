Summary: Command line user interface for the WMS
Name: glite-wms-ui-commands
Version: %{extversion}
Release: %{extage}.%{extdist}
License: ASL 2.0
Vendor: EMI
URL: http://glite.cern.ch/
Group: System Environment/Libraries
BuildArch: %{_arch}
BuildRequires: %{!?extbuilddir: glite-wms-wmproxy-api-cpp-devel,glite-wms-wmproxy-api-cpp,} chrpath, cmake
BuildRequires: %{!?extbuilddir: gridsite-devel, glite-wms-utils-exception-devel,} classads-devel
BuildRequires: %{!?extbuilddir: glite-jobid-api-cpp-devel, glite-jdl-api-cpp-devel,} boost-devel
BuildRequires: %{!?extbuilddir: glite-lb-client-devel, glite-wms-ui-api-python,} libtar-devel
BuildRequires: %{!?extbuilddir: voms-devel, }  zlib-devel
BuildRequires: %{!?extbuilddir:glite-build-common-cpp, } docbook-style-xsl, libxslt, c-ares-devel, libxslt-devel
BuildRequires: globus-common-devel, globus-callout-devel, globus-openssl-devel
BuildRequires: globus-openssl-module-devel, globus-gsi-callback-devel, globus-gsi-cert-utils-devel
BuildRequires: globus-gsi-credential-devel, globus-gsi-openssl-error-devel, globus-gsi-proxy-core-devel
BuildRequires: globus-gsi-proxy-ssl-devel, globus-gsi-sysconfig-devel,globus-gssapi-error-devel
BuildRequires: globus-gssapi-gsi-devel, globus-gss-assist-devel
BuildRequires: globus-ftp-client-devel, globus-ftp-control-devel, libxml2-devel, emi-pkgconfig-compat
Requires: glite-wms-ui-api-python, globus-gass-copy-progs
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%global debug_package %{nil}

%description
Command line user interface for the WMS

%prep
%{!?extbuilddir:%define extbuilddir "-"}
%setup -c -q

%build
if test "x%{extbuilddir}" == "x-" ; then
  cmake -DPREFIX:string=%{buildroot}/usr -DPVER:string=%{version} .
  make
fi

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}
if test "x%{extbuilddir}" == "x-" ; then
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
%dir /usr/share/doc/glite-wms-ui-commands-%{version}/
%doc /usr/share/doc/glite-wms-ui-commands-%{version}/LICENSE
%doc /usr/share/man/man1/*.1.gz
/usr/lib64/libglite_wmsui*
/usr/bin/glite-wms-job-*

%changelog
* %{extcdate} WMS group <wms-support@lists.infn.it> - %{extversion}-%{extage}.%{extdist}
- %{extclog}
