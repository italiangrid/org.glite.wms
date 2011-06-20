#
#       Copyright (c) 2004 on behalf of the EU EGEE Project:
#       The European Organization for Nuclear Research (CERN),
#       Istituto Nazionale di Fisica Nucleare (INFN), Italy
#       Datamat Spa, Italy
#       Centre National de la Recherche Scientifique (CNRS), France
#       CS Systeme d'Information (CSSI), France
#       Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
#       Universiteit van Amsterdam (UvA), Netherlands
#       University of Helsinki (UH.HIP), Finland
#       University of Bergen (UiB), Norway
#       Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
#
#       Authors: Giuseppe Avellino <giuseppe.avellino@datamat.it>
#

AC_PREREQ(2.57)
AC_INIT([GLite WMS wmproxy], [3.3.1])
AC_CONFIG_AUX_DIR([./project])
AM_INIT_AUTOMAKE([1.6.3 subdir-objects])
AC_CONFIG_SRCDIR([src/server/wmproxy.cpp])

# Notices.
AC_COPYRIGHT([Copyright (c) 2004 The EU EGEE Project
See LICENCE file for details
])
AC_REVISION([$Revision$])

#Environment.
WORKDIR=`pwd`
AC_SUBST(WORKDIR)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AM_PROG_CC_C_O
AC_PROG_YACC
AM_PROG_LEX
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_HEADER_TIME
AC_TYPE_SIZE_T
AC_STRUCT_TM

AH_TEMPLATE([WITH_IPV6], [For Ipv6 support])
AC_DEFINE(WITH_IPV6)


# Checks for library functions.
AC_HEADER_STDC
AC_CHECK_FUNCS([memset])
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([ftruncate localtime_r pow strdup strerror strstr])

GLITE_CHECK_LIBDIR
GLITE_CHECK_INITDIR

AC_OPTIMIZE
AC_GLITE_WMS_WSDL

have_boost=no
have_classads=no
have_globus=no
have_expat=no
have_fcgi=no
have_gridsite=no
have_libtar=no
have_zlib=no
have_voms=no
 
AC_BOOST([], have_boost=yes, have_boost=no)
AC_MSG_RESULT(["BOOST found $have_boost"])

AC_CLASSADS([], have_classads=yes, have_classads=no)
AC_MSG_RESULT(["CLASSADS found $have_classads"])

AC_GLOBUS([], have_globus=yes, have_globus=no)
AC_MSG_RESULT(["GLOBUS found $have_globus"])

AC_EXPAT([], have_expat=yes, have_expat=no)
AC_MSG_RESULT(["EXPAT found $have_expat"])

AC_FCGI([], have_fcgi=yes, have_fcgi=no)
AC_MSG_RESULT(["FCGI found $have_fcgi"])

AC_GRIDSITE([], have_gridsite=yes, have_gridsite=no)
AC_MSG_RESULT(["GRIDSITE found $have_gridsite"])

AC_LIBTAR([], have_libtar=yes, have_libtar=no) 
AC_MSG_RESULT(["LIBTAR found $have_libtar"]) 
    
AC_ZLIB([], have_zlib=yes, have_zlib=no) 
AC_MSG_RESULT(["ZLIB found $have_zlib"]) 

GLITE_CHECK_XSLT
AC_ARES

if test "$have_globus" = "no"; then
  AC_MSG_RESULT(["Please verify GLOBUS package"])
fi

if test "$have_boost" = "no"; then
  AC_MSG_RESULT(["Please verify BOOST package"])
fi

if test "$have_classads" = "no"; then
  AC_MSG_RESULT(["Please verify CLASSADS package"])
fi

if test "$have_expat" = "no"; then
  AC_MSG_RESULT(["Please verify EXPAT package"])
fi

if test "$have_fcgi" = "no"; then
  AC_MSG_RESULT(["Please verify FCGI package"])
fi

if test "$have_gridiste" = "no"; then
  AC_MSG_RESULT(["Please verify GRIDSITE package"])
fi

if test "$have_libtar" = "no"; then 
  AC_MSG_RESULT(["Please verify LIBTAR package"]) 
fi 
    
if test "$have_zlib" = "no"; then 
  AC_MSG_RESULT(["Please verify ZLIB package"]) 
fi 


if test "$have_globus" = "no"; then
  AC_MSG_ERROR([Cannot continue building])
fi

if test "$have_boost" = "no"; then
  AC_MSG_ERROR([Cannot continue building])
fi

if test "$have_classads" = "no"; then
  AC_MSG_ERROR([Cannot continue building])
fi

if test "$have_expat" = "no"; then
  AC_MSG_ERROR([Cannot continue building])
fi

if test "$have_fcgi" = "no"; then
  AC_MSG_ERROR([Cannot continue building])
fi

if test "$have_gridsite" = "no"; then
  AC_MSG_ERROR([Cannot continue building])
fi

if test "$have_libtar" = "no"; then 
  AC_MSG_ERROR([Cannot continue building]) 
fi 
    
if test "$have_zlib" = "no"; then 
  AC_MSG_ERROR([Cannot continue building]) 
fi 

AC_VOMS([], have_voms=yes, have_voms=no)
if test "$have_voms" = "no"; then
  AC_MSG_ERROR([Cannot continue building])
fi

GLITE_CHECK_JOBID

have_glite_wms_common=no
have_glite_wmsutils=no
have_glite_jdl=no
have_glite_lb=no
have_glite_voms=no
have_glite_renewal=no                                                                                
have_glite_wms_purger=no

AC_GLITE_WMS_COMMON([], have_glite_wms_common=yes, have_glite_wms_common=no)
AC_MSG_RESULT(["GLITE_WMS_COMMON found $have_glite_wms_common"])

AC_GLITE_WMSUTILS([], have_glite_wmsutils=yes, have_glite_wmsutils=no)
AC_MSG_RESULT(["GLITE_WMSUTILS found $have_glite_wmsutils"])

AC_GLITE_JDL([], have_glite_jdl=yes, have_glite_jdl=no)
AC_MSG_RESULT(["GLITE_JDL found $have_glite_jdl"])

AC_GLITE_LB([], have_glite_lb=yes, have_glite_lb=no)
AC_MSG_RESULT(["GLITE_LB found $have_glite_lb"])

AC_VOMS([], have_glite_voms=yes, have_glite_voms=no)
AC_MSG_RESULT(["VOMS found $have_glite_voms"])

AC_PROXYRENEWAL([], have_glite_renewal=yes, have_glite_renewal=no)
AC_MSG_RESULT(["PROXY RENEWAL found $have_glite_renewal"])

AC_GLITE_WMS_PURGER([], have_glite_wms_purger=yes, have_glite_wms_purger=no)
AC_MSG_RESULT(["GLITE_WMS_PURGER found $have_glite_wms_purger"])

AC_GLITE_SERVICE_DISCOVERY
AC_GSOAP

AC_SEC_LCAS
AC_SEC_LCMAPS
AC_SEC_LCMAPS_WITHOUT_GSI

PKG_CHECK_MODULES(ARGUS_PEP_API_C, libargus-pep, , AC_MSG_ERROR(["ARGUS_PEP_API_C not found"]))

# Configuration items
AC_PREFIX_DEFAULT([/opt/glite])
AM_CONFIG_HEADER([src/autogen/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([config/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/utilities/Makefile])
AC_CONFIG_FILES([src/eventlogger/Makefile])
AC_CONFIG_FILES([src/authorizer/Makefile])
AC_CONFIG_FILES([src/commands/Makefile])
AC_CONFIG_FILES([src/dirmanager/Makefile])
AC_CONFIG_FILES([src/server/Makefile])
AC_CONFIG_FILES([src/tools/Makefile])
AC_CONFIG_FILES([src/scripts/Makefile])
#AC_CONFIG_FILES([test/Makefile])
#AC_CONFIG_FILES([interface/Makefile])
AC_CONFIG_FILES([wmproxy.dox])
AC_CONFIG_FILES([src/scripts/glite-wms-wmproxy-purge-proxycache])

AC_OUTPUT


