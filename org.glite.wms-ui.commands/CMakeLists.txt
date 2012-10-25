project(wms-ui-commands CXX)
cmake_minimum_required(VERSION 2.6)

if(NOT DEFINED PVER)
  message(WARNING " Undefined version number, selected 0.0.0. Please use
-DPVER=<version> to customize")
    set(PVER "0.0.0")
endif(NOT DEFINED PVER)

if(NOT DEFINED CLI_VERSION)
  message(WARNING " Undefined CLI_VERSION label. 1.0 will be used")
  set(CLI_VERSION "0.0.0")
endif(NOT DEFINED CLI_VERSION)

if(NOT DEFINED PREFIX)
    message(WARNING " Undefined prefix, selected /usr/local")
    set(PREFIX "/usr/local")
endif(NOT DEFINED PREFIX)

if(NOT DEFINED PKG_INSTALL_PREFIX)
    set(PKG_INSTALL_PREFIX ${PREFIX})
endif(NOT DEFINED PKG_INSTALL_PREFIX)

if( ${CMAKE_VERSION} VERSION_LESS "2.8.2" )
    if( IS_DIRECTORY "/usr/lib64" )
        set(LIBDIR "lib64")
    else( IS_DIRECTORY "/usr/lib64" )
        set(LIBDIR "lib")
    endif( IS_DIRECTORY "/usr/lib64" )
else( ${CMAKE_VERSION} VERSION_LESS "2.8.2" )
    if( IS_DIRECTORY "/usr/lib64" AND NOT IS_SYMLINK "/usr/lib64" )
        set(LIBDIR "lib64")
    else( IS_DIRECTORY "/usr/lib64" AND NOT IS_SYMLINK "/usr/lib64" )
        set(LIBDIR "lib")
    endif( IS_DIRECTORY "/usr/lib64" AND NOT IS_SYMLINK "/usr/lib64" )
endif( ${CMAKE_VERSION} VERSION_LESS "2.8.2" )

if(NOT DEFINED SYSCONFIGDIR)
    set(SYSCONFIGDIR "etc/")
endif(NOT DEFINED SYSCONFIGDIR)


set(BINDIR "bin")

set(LIBEXEC "libexec")



set(ENV{PKG_CONFIG_PATH}
"${PREFIX}/lib/pkgconfig/:${PREFIX}/lib64/pkgconfig/:$ENV{PKG_CONFIG_PATH}")

if(DEFINED ADDITIONAL_PKG_CONFIG_PATH)
  set(ENV{PKG_CONFIG_PATH}
"${ADDITIONAL_PKG_CONFIG_PATH}:$ENV{PKG_CONFIG_PATH}")
endif(DEFINED ADDITIONAL_PKG_CONFIG_PATH)

find_package(PkgConfig REQUIRED)
find_package(Perl REQUIRED)
find_program(Make REQUIRED)

pkg_check_modules(GRIDSITE_GLOBUS REQUIRED gridsite-globus)
pkg_check_modules(GRIDSITE_OPENSSL REQUIRED gridsite-openssl)
pkg_check_modules(GSOAP REQUIRED gsoap)
pkg_check_modules(GSOAP_PLUSPLUS REQUIRED gsoap++)
pkg_check_modules(GSOAP_SSL REQUIRED gsoapssl)
pkg_check_modules(GSOAP_SSL_PLUSPLUS REQUIRED gsoapssl++)
pkg_check_modules(JDL_API_CPP REQUIRED jdl-api-cpp)
pkg_check_modules(CLASSAD REQUIRED emi-classad-ns)
pkg_check_modules(JOBID REQUIRED emi-jobid)
pkg_check_modules(LB_CLIENT REQUIRED emi-lb-client)
pkg_check_modules(LB_CLIENTPP REQUIRED emi-lb-clientpp)

pkg_check_modules(CLASSAD_UTILS REQUIRED classad-utils)
pkg_check_modules(JOBMAN_EXCEPTION REQUIRED jobman-exception)
pkg_check_modules(VOMS REQUIRED voms-2.0)
pkg_check_modules(GLOBUS_GSS_ASSIST REQUIRED globus-gss-assist)
pkg_check_modules(BOOST_REGEX emi-boost-regex)
pkg_check_modules(BOOST_FILESYSTEM emi-boost-filesystem)
pkg_check_modules(LIBXSLT REQUIRED libexslt)
pkg_check_modules(LIBXSLTDEV REQUIRED libxslt)
pkg_check_modules(WMPROXY_API_CPP REQUIRED wmproxy-api-cpp)


if(NOT EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl"
   AND NOT EXISTS "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")
  message(FATAL_ERROR "*** Missing docbook xsl style sheet required to parse man pages")
endif(NOT EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl"
   AND NOT EXISTS "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")

if(EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")
  set(DOCBOOK_MAN_PAGES_STYLE "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")
endif(EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")

if(EXISTS "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")
  set(DOCBOOK_MAN_PAGES_STYLE "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")
endif(EXISTS "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")


#message( "\nDOCBOOK_MAN_PAGES_STYLE=${DOCBOOK_MAN_PAGES_STYLE}\n")


if(DEFINED BUILD_DBG)
  add_definitions( -pipe -g -O0 -Wall -DWANT_NAMESPACES -DWANT_CLASSAD_NAMESPACE -DWMP_CLT_SOFTWARE_VERSION="${CLI_VERSION}")
  
else(DEFINED BUILD_DBG)
    add_definitions( -pipe -O2 -Wall -DWANT_NAMESPACES -DWANT_CLASSAD_NAMESPACE -DWMP_CLT_SOFTWARE_VERSION="${CLI_VERSION}")
endif(DEFINED BUILD_DBG)

add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(config)
