Summary: Java libraries for the WM Proxy service
Name: glite-wms-wmproxy-api-java
Version:
Release:
License: Apache License 2.0
Vendor: EMI
Packager: WMS group <wms-support@lists.infn.it>
URL: http://glite.cern.ch/
Group: System Environment/Libraries
BuildArch: noarch
Requires: vomsjapi
Requires: emi-trustmanager-axis
Requires: emi-delegation-java
BuildRequires: ant
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz


%description
Java libraries for the WM Proxy service

%prep
 

%setup -c

%build
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  printf "dist.location=%{buildroot}/usr
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
  cp %{extbuilddir}/usr/share/doc/glite-wms-wmproxy-api-java/LICENSE %{buildroot}/usr/share/doc/glite-wms-wmproxy-api-java-%{version}
  mkdir -p %{buildroot}/%{_javadocdir}/%{name}
  cp -R %{extbuilddir}/usr/share/doc/glite-wms-wmproxy-api-java/html %{buildroot}/%{_javadocdir}/%{name}
fi
 

%clean
rm -rf %{buildroot} 

%files
%defattr(-,root,root)
/usr/share/java/glite-wms-wmproxy-api-java.jar
%dir /usr/share/doc/glite-wms-wmproxy-api-java-%{version}/
%doc /usr/share/doc/glite-wms-wmproxy-api-java-%{version}/LICENSE

%changelog


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

