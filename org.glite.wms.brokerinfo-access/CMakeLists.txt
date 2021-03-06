project(wms-brokerinfo-access CXX)
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
"${PREFIX}/lib/pkgconfig/:${PREFIX}/lib64/pkgconfig/:$ENV{PKG_CONFIG_PATH}:/usr/lib64/pkgconfig:/usr/lib/pkgconfig")

if(DEFINED ADDITIONAL_PKG_CONFIG_PATH)
  set(ENV{PKG_CONFIG_PATH}
"${ADDITIONAL_PKG_CONFIG_PATH}:$ENV{PKG_CONFIG_PATH}")
endif(DEFINED ADDITIONAL_PKG_CONFIG_PATH)

find_package(PkgConfig REQUIRED)
find_package(Perl REQUIRED)
find_program(Make REQUIRED)
pkg_check_modules(CLASSAD REQUIRED emi-classad-ns)
pkg_check_modules(LIBXSLT REQUIRED libxslt)

if(NOT EXISTS "/usr/bin/doxygen")
  message(FATAL_ERROR "*** Missing doxygen")
endif(NOT EXISTS "/usr/bin/doxygen")

#if(NOT EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl"
#   AND NOT EXISTS "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")
#  message(FATAL_ERROR "*** Missing docbook xsl style sheet required to parse man pages")
#endif(NOT EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl"
#   AND NOT EXISTS "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")

#if(EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")
#  set(DOCBOOK_MAN_PAGES_STYLE "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")
#endif(EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")

#if(EXISTS "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")
#  set(DOCBOOK_MAN_PAGES_STYLE "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")
#endif(EXISTS "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")


if(DEFINED BUILD_DBG)
   add_definitions( -pipe -g -O0 -Wall ${CLASSAD_CFLAGS} )
else(DEFINED BUILD_DBG)
   add_definitions( -pipe -O2 -Wall ${CLASSAD_CFLAGS} )
endif(DEFINED BUILD_DBG)

install(FILES
	LICENSE
	DESTINATION ${PREFIX}/share/doc/glite-wms-brokerinfo-access-${PVER}/
)

add_subdirectory(src)
add_subdirectory(doc)
