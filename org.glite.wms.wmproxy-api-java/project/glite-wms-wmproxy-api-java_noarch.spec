Summary: Java libraries for the WM Proxy service
Name: glite-wms-wmproxy-api-java
Version: %{extversion}
Release: %{extage}.%{extdist}
License: Apache Software License
Vendor: EMI
URL: http://glite.cern.ch/
Group: System Environment/Libraries
BuildArch: noarch
Requires: vomsjapi
Requires: emi-trustmanager-axis
Requires: emi-delegation-java
BuildRequires: %{!?extbuilddir: glite-wms-wmproxy-interface, emi-delegation-java,} ant
BuildRequires: %{!?extbuilddir: emi-trustmanager-axis, vomsjapi,} axis1.4
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz


%description
Java libraries for the WM Proxy service

%prep
 

%setup -c -q

%build
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  printf "dist.location=%{buildroot}
stage.location=
docs-installdir=%{buildroot}/%{_javadocdir}/%{name}
module.version=%{version}">.configuration.properties
  ant
fi

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  ant install
else
  mkdir -p %{buildroot}/usr/share/java
  cp %{extbuilddir}/usr/share/java/*.jar %{buildroot}/usr/share/java
  mkdir -p %{buildroot}/usr/share/doc/glite-wms-wmproxy-api-java-%{version}
  cp %{extbuilddir}/usr/share/doc/glite-wms-wmproxy-api-java-%{version}/LICENSE %{buildroot}/usr/share/doc/glite-wms-wmproxy-api-java-%{version}
  #mkdir -p %{buildroot}/%{_javadocdir}/%{name}
  #cp -R %{extbuilddir}/usr/share/doc/glite-wms-wmproxy-api-java/html %{buildroot}/%{_javadocdir}/%{name}
  mkdir -p %{buildroot}/%{_javadocdir}/%{name}
  ln -s %{extbuilddir}/usr/share/doc/glite-wms-wmproxy-api-java/html %{buildroot}/%{_javadocdir}/%{name}/html
fi
 

%clean
rm -rf %{buildroot} 

%files
%defattr(-,root,root)
/usr/share/java/glite-wms-wmproxy-api-java.jar
%dir /usr/share/doc/glite-wms-wmproxy-api-java-%{version}/
%doc /usr/share/doc/glite-wms-wmproxy-api-java-%{version}/LICENSE



%package doc
Summary: Documentation files for Job Description Language library
Group: Documentation
Requires: %{name}

%description doc
Documentation files for dealing with Job Description Language

%files doc
%defattr(-,root,root)
%dir %{_javadocdir}/%{name}/html/
%dir %{_javadocdir}/%{name}/html/resources/
%doc %{_javadocdir}/%{name}/html/resources/inherit.gif

%doc %{_javadocdir}/%{name}/html/*.html
%doc %{_javadocdir}/%{name}/html/stylesheet.css
%doc %{_javadocdir}/%{name}/html/package-list
%dir %{_javadocdir}/%{name}/html/org/
%dir %{_javadocdir}/%{name}/html/org/glite/
%dir %{_javadocdir}/%{name}/html/org/glite/wms/
%dir %{_javadocdir}/%{name}/html/org/glite/wms/wmproxy/
%doc %{_javadocdir}/%{name}/html/org/glite/wms/wmproxy/*.html
%dir %{_javadocdir}/%{name}/html/org/glite/wms/wmproxy/class-use/
%doc %{_javadocdir}/%{name}/html/org/glite/wms/wmproxy/class-use/*.html


%changelog
* %(date +"%%a %%b %%d %%Y") WMS group <wms-support@lists.infn.it> - %{version}-%{release}
- %{extclog}

