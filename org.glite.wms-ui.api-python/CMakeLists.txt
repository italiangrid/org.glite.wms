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
pkg_check_modules(JDL_API_CPP REQUIRED jdl-api-cpp)
pkg_check_modules(CLASSAD REQUIRED emi-classad-ns)
pkg_check_modules(JOBID REQUIRED emi-jobid)
pkg_check_modules(LB_CLIENT REQUIRED emi-lb-client)
pkg_check_modules(CLASSAD_UTILS REQUIRED classad-utils)
pkg_check_modules(JOBMAN_EXCEPTION REQUIRED jobman-exception)
pkg_check_modules(VOMS REQUIRED voms-2.0)
pkg_check_modules(GLOBUS_GSS_ASSIST REQUIRED globus-gss-assist)
pkg_check_modules(BOOST_FILESYSTEM emi-boost-filesystem)


if(NOT EXISTS "/usr/bin/python")
  message(FATAL_ERROR "\n\n*** ERROR: Python installation missing. Cannot build this component\n\n")
endif(NOT EXISTS "/usr/bin/python" )

if(NOT EXISTS "/usr/bin/python-config")
  message(FATAL_ERROR "\n\n*** ERROR: Python devel installation missing. Cannot build this component\n\n")
endif(NOT EXISTS "/usr/bin/python-config" )

EXEC_PROGRAM(python ARGS
  "-c 'import platform; print platform.python_version()' | awk -F'.' '{print $1\".\"$2}' "
  OUTPUT_VARIABLE
  PYTHON_VERSION)

message("\nPYTHON_VERSION=${PYTHON_VERSION}\n")

if(EXISTS "/usr/bin/swig" 
    OR EXISTS "/bin/swig"
    OR EXISTS "/usr/local/bin/swig"
    OR EXISTS "/opt/local/bin/swig")
  set(SWIG_OK 1)
  else(EXISTS "/usr/bin/swig" 
    OR EXISTS "/bin/swig"
    OR EXISTS "/usr/local/bin/swig"
    OR EXISTS "/opt/local/bin/swig")
  message(FATAL_ERROR "\n\n*** ERROR: Swig installation missing. Cannot build this component\n\n")
endif(EXISTS "/usr/bin/swig" 
    OR EXISTS "/bin/swig"
    OR EXISTS "/usr/local/bin/swig"
    OR EXISTS "/opt/local/bin/swig")

EXEC_PROGRAM(python-config ARGS --includes
  OUTPUT_VARIABLE PYTHON_INCLUDES)

if(DEFINED BUILD_DBG)
  add_definitions( -pipe -g -O0 -Wall ${CLASSAD_CFLAGS} ${PYTHON_INCLUDES} ${JOBMAN_EXCEPTION_CFLAGS})

else(DEFINED BUILD_DBG)
    add_definitions( -pipe -O2 -Wall ${CLASSAD_CFLAGS} ${PYTHON_INCLUDES} ${JOBMAN_EXCEPTION_CFLAGS})
endif(DEFINED BUILD_DBG)

add_subdirectory(src)
