Summary: SAM sensors for certification testbed
Name: lcg-sam-client-sensors-ctb
Version: 1.0.0
Vendor: LCG/CERN
Release: 1
License: LCG
Group: LCG
Source: %{name}.src.tgz
BuildArch: noarch
Packager: project-lcg-deployment-bitface@cern.ch
BuildRoot: %{_tmppath}/%{name}-%{version}-build
Prefix: /opt/glite

%description
 SAM sensors for certification testbed
%prep

%setup -c

%build
make install prefix=%{buildroot}%{prefix}

%files
%defattr(-,root,root)
%{prefix}/client/sensors/DPM/

%clean
rm -rf $RPM_BUILD_ROOT

