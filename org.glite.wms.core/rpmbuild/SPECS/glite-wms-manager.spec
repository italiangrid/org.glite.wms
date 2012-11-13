Summary: Core logic for the Workload Management System
Name: glite-wms-manager
Version: 3.5.0
Release: 0.sl6
License: Apache Software License
Vendor: EMI
URL: http://glite.cern.ch/
Group: Applications/Internet
BuildArch: %{_arch}
Requires: glite-wms-configuration glite-wms-helper glite-wms-ism glite-wms-purger glite-wms-common glite-lb-client
Requires(post): chkconfig
Requires(preun): chkconfig
Requires(preun): initscripts
BuildRequires: chrpath, libtool, boost-devel, c-ares-devel, classads-devel, globus-ftp-client-devel, globus-ftp-control-devel, docbook-style-xsl
BuildRequires: glite-build-common-cpp, glite-jobid-api-c-devel, 
BuildRequires: %{!?extbuilddir: glite-wms-common-devel, glite-wms-ism-devel, glite-wms-helper-devel, glite-wms-purger-devel, } glite-px-proxyrenewal-devel
BuildRequires: glite-lb-client-devel, glite-jobid-api-cpp-devel, libxslt
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%global debug_package %{nil}

%description
Core logic for the Workload Management System

%prep
 
%setup -c -q

%build
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  ./configure --prefix=%{buildroot}/usr --sysconfdir=%{buildroot}/etc --disable-static PVER=%{version}
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

chrpath --delete %{buildroot}/usr/bin/glite-wms-workload_manager
strip -s %{buildroot}/usr/bin/glite-wms-workload_manager
chrpath --delete %{buildroot}/usr/bin/glite-wms-query-job-state-transitions
strip -s %{buildroot}/usr/bin/glite-wms-query-job-state-transitions
export QA_SKIP_BUILD_ROOT=yes

%clean
rm -rf %{buildroot}

%post
/sbin/chkconfig --add glite-wms-wm 

%preun
if [ $1 -eq 0 ] ; then
    /sbin/service glite-wms-wm stop >/dev/null 2>&1
    /sbin/chkconfig --del glite-wms-wm
fi

%files
%defattr(-,root,root)
/etc/rc.d/init.d/glite-wms-wm
/usr/bin/glite-wms-workload_manager
/usr/bin/glite-wms-stats
/usr/bin/glite-wms-query-job-state-transitions
%dir /usr/share/doc/glite-wms-manager-%{version}/
%doc /usr/share/doc/glite-wms-manager-%{version}/LICENSE
%doc /usr/share/man/man1/glite-wms-*.1.gz

%changelog
* Fri Nov 09 2012 WMS group <wms-support@lists.infn.it> - 3.5.0-0.sl6
- Bug fixing
