project(wms-wmproxy-api-cpp CXX)
cmake_minimum_required(VERSION 2.6)

if(NOT DEFINED PVER)
    message(WARNING " Undefined version number, selected 9.9.9. Please use
-DPVER=<version> to customize")
    set(PVER "9.9.9")
endif(NOT DEFINED PVER)

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
message("-- checking for module 'emi-delegation-interface'")
if( EXISTS "/var/lib/delegation-interface/interface/www.gridsite.org-delegation-2.0.0.wsdl")
  message("--   found emi-delegation-interface")
else( message("*** ERROR: Required emi-delegation-interface package not found") )
endif( EXISTS "/var/lib/delegation-interface/interface/www.gridsite.org-delegation-2.0.0.wsdl")

#add_definitions( -pipe -DWANT_NAMESPACES -DWANT_CLASSAD_NAMESPACE )
add_definitions( -pipe )

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -DWITH_IPV6 -DUSE_GSOAP_PARSER -UHAVE_CONFIG_H -I/usr/include/gsoap")

set(GSOAP_VERSION "")
if(EXISTS "/usr/bin/dpkg")
  EXEC_PROGRAM(/usr/bin/dpkg ARGS -s gsoap|grep Version|/usr/bin/awk '{print $2}'
    OUTPUT_VARIABLE GSOAP_VERSION)
endif(EXISTS "/usr/bin/dpkg")

#message("\nPREFIX=${PREFIX}\nCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}\nLIBDIR=${LIBDIR}\nBINDIR=${BINDIR}\n")
message("\n\nGSOAP_VERSION=${GSOAP_VERSION}\n\n")
add_subdirectory(src)
add_subdirectory(interface)
