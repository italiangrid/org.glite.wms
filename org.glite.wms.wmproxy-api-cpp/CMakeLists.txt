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
"${PREFIX}/lib/pkgconfig/:${PREFIX}/lib64/pkgconfig/:$ENV{PKG_CONFIG_PATH}/usr/lib64/pkgconfig:/usr/lib/pkgconfig")

if(DEFINED ADDITIONAL_PKG_CONFIG_PATH)
  set(ENV{PKG_CONFIG_PATH}
"${ADDITIONAL_PKG_CONFIG_PATH}:$ENV{PKG_CONFIG_PATH}")
endif(DEFINED ADDITIONAL_PKG_CONFIG_PATH)

#message("PKG_CONFIG_PATH=$ENV{PKG_CONFIG_PATH}\n\n")

find_package(PkgConfig REQUIRED)
find_package(Perl REQUIRED)
find_program(Make REQUIRED)
#pkg_check_modules(GRIDSITE_GLOBUS REQUIRED gridsite-globus)
pkg_check_modules(GRIDSITE_OPENSSL REQUIRED gridsite-openssl)
pkg_check_modules(GSOAP REQUIRED gsoap)
pkg_check_modules(GSOAP_PLUSPLUS REQUIRED gsoap++)
pkg_check_modules(GSOAP_SSL REQUIRED gsoapssl)
pkg_check_modules(GSOAP_SSL_PLUSPLUS REQUIRED gsoapssl++)

#if(NOT EXISTS "/usr/bin/lsb_release")
#  message(FATAL_ERROR "\n\nMissing /usr/bin/lsb_release required to determine Linux Release\n\n")
#endif(NOT EXISTS "/usr/bin/lsb_release")

#EXEC_PROGRAM(/usr/bin/lsb_release ARGS -d |/usr/bin/awk '{print $2}'
#  OUTPUT_VARIABLE LINUX_VENDOR)

#EXEC_PROGRAM(/usr/bin/lsb_release 
#  ARGS 
#  -r |/usr/bin/awk '{print $2}'|/usr/bin/awk -F"." '{print $1}' 
#  OUTPUT_VARIABLE LINUX_VENDOR_VERSION)



if(EXISTS "/etc/debian_version")
  set(ONDEB "1")
endif(EXISTS "/etc/debian_version")



if(EXISTS "/etc/redhat-release")
  set(ONRH "1")
endif(EXISTS "/etc/redhat-release")



if(DEFINED ONDEB)
EXEC_PROGRAM(cat
  ARGS /etc/debian_version | /usr/bin/awk -F"." '{print $1}'
  OUTPUT_VARIABLE DEB_VER)
endif(DEFINED ONDEB)



if(DEFINED ONRH)
EXEC_PROGRAM(cat
  ARGS  /etc/redhat-release | /usr/bin/awk '{print $(NF-1)}' | /usr/bin/awk -F"." '{print $1}'
  OUTPUT_VARIABLE RH_VER)
endif(DEFINED ONRH)



if(DEFINED ONDEB)
  EXEC_PROGRAM(/usr/bin/dpkg 
    ARGS -s gsoap|grep Version|/usr/bin/awk '{print $2}'
    OUTPUT_VARIABLE GSOAP_VERSION)
endif(DEFINED ONDEB)




if(DEFINED BUILD_DBG)

  add_definitions( -pipe -g -O0 -Wall -DWITH_OPENSSL -DWITH_DOM -DWITH_GZIP -DHAVE_CONFIG_H -DWITH_IPV6 -DUSE_GSOAP_PARSER -DWITH_COOKIES -DWITH_GZIP -I/usr/include)

else(DEFINED BUILD_DBG)

  add_definitions( -pipe -O2 -Wall -DWITH_OPENSSL -DWITH_DOM -DWITH_GZIP -DHAVE_CONFIG_H -DWITH_IPV6 -DUSE_GSOAP_PARSER -DWITH_COOKIES -DWITH_GZIP -I/usr/include)

endif(DEFINED BUILD_DBG)








#if("${LINUX_VENDOR}" STREQUAL "Debian")
  
#endif("${LINUX_VENDOR}" STREQUAL "Debian")


if(DEFINED ONRH)
  
  if("${RH_VER}" STREQUAL "5")
    add_definitions( -DGSOAP_VERSION_NUM=20713 )
  endif("${RH_VER}" STREQUAL "5")

  if("${RH_VER}" STREQUAL "6")
    add_definitions( -DGSOAP_VERSION_NUM=20716 )
  endif("${RH_VER}" STREQUAL "6")

endif(DEFINED ONRH)


message("\n\nGSOAP_VERSION=${GSOAP_VERSION}\n\n")

install(FILES
        LICENSE
	CHANGES
        DESTINATION ${PREFIX}/share/doc/glite-wms-wmproxy-api-cpp-${PVER}/
)

add_custom_command(OUTPUT 
        ${CMAKE_CURRENT_SOURCE_DIR}/autodoc/html/index.html
        COMMAND 
        SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}" PROJECT="glite-wms-wmproxy-api-cpp" DOCDIR="autodoc" VERSION="${PVER}" PERL_PATH="/usr/bin/perl" HAVE_DOT="NO" GENERATE_MAN="NO" GENERATE_RTF="NO" GENERATE_XML="NO" GENERATE_HTMLHELP="NO" GENERATE_CHI="NO" GENERATE_HTML="YES" GENERATE_LATEX="NO" /usr/bin/doxygen ${CMAKE_CURRENT_SOURCE_DIR}/project/doxygen/doxygen.cfg
)

add_custom_target(DOXPROC ALL DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/autodoc/html/index.html
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/autodoc/html
        DESTINATION
        ${PREFIX}/share/doc/glite-wms-wmproxy-api-cpp
)


add_subdirectory(src)
add_subdirectory(interface)
