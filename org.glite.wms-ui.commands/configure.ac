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

AC_PREREQ(2.57)
AC_INIT([GLite WMS UI Commands], [3.3.3])
AC_CONFIG_AUX_DIR([./project])
AM_INIT_AUTOMAKE([1.6.3 subdir-objects])

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
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_UID_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_HEADER_STDC
AC_CHECK_FUNCS([memset])
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([ftruncate localtime_r pow strdup strerror strstr])

GLITE_CHECK_LIBDIR
AC_GLITE_DOCBOOK
AC_OPTIMIZE

PKG_CHECK_MODULES(GSOAP, gsoapssl++, , AC_MSG_ERROR(["GSOAP not found"]))
AC_GSOAP_EXTRAS

PKG_CHECK_MODULES(ARES, libcares, , AC_MSG_ERROR(["ARES not found"]))

AC_BOOST([], AC_MSG_RESULT(["BOOST ok"]), AC_MSG_ERROR(["BOOST not found"]))

PKG_CHECK_MODULES(CLASSAD, emi-classad-ns, , AC_MSG_ERROR(["CLASSADS not found"]))

#AC_GRIDSITE([], AC_MSG_RESULT(["GRIDSITE ok"]), AC_MSG_ERROR(["GRIDSITE not found"]))
PKG_CHECK_MODULES(GRIDSITE_OPENSSL, emi-gridsite-openssl, , AC_MSG_ERROR(["GRIDSITE not found"]))

AC_ZLIB([], AC_MSG_RESULT(["ZLIB ok"]), AC_MSG_ERROR(["ZLIB not found"]))

AC_LIBTAR([], AC_MSG_RESULT(["LIBTAR ok"]), AC_MSG_ERROR(["LIBTAR not found"]))

PKG_CHECK_MODULES(GLITE_WMSUTILS_EXCEPTION, jobman-exception, , AC_MSG_ERROR(["GLITE_WMSUTILS_EXCEPTION not found"]))

PKG_CHECK_MODULES(GLITE_WMSUTILS_CLASSADS, classad-utils, , AC_MSG_ERROR(["GLITE_WMSUTILS_CLASSADS not found"]))

#GLITE_CHECK_JOBID([], AC_MSG_RESULT(["GLITE_JOBID ok"]), AC_MSG_ERROR(["GLITE_JOBID not found"]))
PKG_CHECK_MODULES(GLITE_JOBID, emi-jobid, ,AC_MSG_ERROR(["GLITE_JOBID not found"]))

PKG_CHECK_MODULES(GLITE_JDL, jdl-api-cpp, , AC_MSG_ERROR(["GLITE_JDL not found"]))

PKG_CHECK_MODULES(GLITE_LB_COMMON, emi-lb-common, , AC_MSG_ERROR(["GLITE_LB_COMMON not found"]))
PKG_CHECK_MODULES(GLITE_LB_CLIENT, emi-lb-client, , AC_MSG_ERROR(["GLITE_LB_CLIENT not found"]))
PKG_CHECK_MODULES(GLITE_LB_CLIENTPP, emi-lb-clientpp, , AC_MSG_ERROR(["GLITE_LB_CLIENTPP not found"]))


PKG_CHECK_MODULES(GLITE_WMS_WMPROXY, wmproxy-api-cpp, , AC_MSG_ERROR(["GLITE_WMS_WMPROXY not found"]))

PKG_CHECK_MODULES(VOMS, voms-2.0, , AC_MSG_ERROR(["VOMS not found"]))


# Configuration items
AC_PREFIX_DEFAULT([/usr])
AM_CONFIG_HEADER([src/autogen/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/utilities/Makefile])
AC_CONFIG_FILES([src/services/Makefile])
AC_CONFIG_FILES([src/lbclient/Makefile])
AC_CONFIG_FILES([src/commands/Makefile])
AC_CONFIG_FILES([config/Makefile])
AC_CONFIG_FILES([config/glite_wmsui_cmd_err.conf])
AC_CONFIG_FILES([doc/Makefile])
#AC_CONFIG_FILES([wms-client.dox])


AC_OUTPUT


