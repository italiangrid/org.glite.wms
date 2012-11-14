project(wms-wmproxy-api-cpp CXX)
cmake_minimum_required(VERSION 2.6)

if(NOT DEFINED PVER)
    message(WARNING " Undefined version number, selected 0.0.0. Please use
-DPVER=<version> to customize")
    set(PVER "0.0.0")
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
else( EXISTS "/var/lib/delegation-interface/interface/www.gridsite.org-delegation-2.0.0.wsdl"))
  message("*** ERROR: Required emi-delegation-interface package not found") )
endif( EXISTS "/var/lib/delegation-interface/interface/www.gridsite.org-delegation-2.0.0.wsdl")



if(DEFINED BUILD_DBG)
#  add_definitions( -pipe -g -O0 -Wall -DWITH_IPV6 -DUSE_GSOAP_PARSER -DWITH_DOM -DWITH_COOKIES -DWITH_GZIP -DWITH_OPENSSL -DHAVE_CONFIG_H -I/usr/include/gsoap)
   add_definitions( -pipe -g -O0 -Wall ${GSOAP_SSL_PLUSPLUS_CFLAGS} -DWITH_GZIP -DHAVE_CONFIG_H -DWITH_IPV6 -DUSE_GSOAP_PARSER -DWITH_COOKIES -DWITH_GZIP -I/usr/include/gsoap)
else(DEFINED BUILD_DBG)
#    add_definitions( -pipe -O2 -Wall -DWITH_IPV6 -DUSE_GSOAP_PARSER -DWITH_DOM -DWITH_COOKIES -DWITH_GZIP -DWITH_OPENSSL -DHAVE_CONFIG_H -I/usr/include/gsoap)
   add_definitions( -pipe -O2 -Wall ${GSOAP_SSL_PLUSPLUS_CFLAGS} -DWITH_GZIP -DHAVE_CONFIG_H -DWITH_IPV6 -DUSE_GSOAP_PARSER -DWITH_COOKIES -DWITH_GZIP -I/usr/include/gsoap)
endif(DEFINED BUILD_DBG)



set(GSOAP_VERSION "")
if(EXISTS "/usr/bin/dpkg")
  EXEC_PROGRAM(/usr/bin/dpkg ARGS -s gsoap|grep Version|/usr/bin/awk '{print $2}'
    OUTPUT_VARIABLE GSOAP_VERSION)
endif(EXISTS "/usr/bin/dpkg")

#message("\nPREFIX=${PREFIX}\nCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}\nLIBDIR=${LIBDIR}\nBINDIR=${BINDIR}\n")
message("\n\nGSOAP_VERSION=${GSOAP_VERSION}\n\n")
add_subdirectory(src)
add_subdirectory(interface)
