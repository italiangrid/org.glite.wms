Summary: SAM sensors for certification testbed
Name: lcg-sam-client-sensors-ctb
Version: 0.1
Vendor: LCG/CERN
Release: 19
License: LCG
Group: LCG
Source: %{name}.src.tgz
BuildArch: noarch
Packager: project-lcg-deployment-bitface@cern.ch
BuildRoot: %{_tmppath}/%{name}-%{version}-build
Prefix: /opt/lcg/same/client-ctb

%description
 SAM sensors for certification testbed
%prep

%setup -c

%build
make install prefix=%{buildroot}%{prefix}

%files
%defattr(-,root,root)
%{prefix}/

%clean
rm -rf $RPM_BUILD_ROOT

