## Copyright (c) Members of the EGEE Collaboration. 2009. 
## See http://www.eu-egee.org/partners/ for details on the copyright holders.  

## Licensed under the Apache License, Version 2.0 (the "License"); 
## you may not use this file except in compliance with the License. 
## You may obtain a copy of the License at 
##     http://www.apache.org/licenses/LICENSE-2.0 
## Unless required by applicable law or agreed to in writing, software 
## distributed under the License is distributed on an "AS IS" BASIS, 
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
## See the License for the specific language governing permissions and 
## limitations under the License.

AC_PREREQ(2.57)
AC_INIT([GLite WMS Core], [3.5.0])
AC_CONFIG_AUX_DIR([./project])
AM_INIT_AUTOMAKE([1.6.3 subdir-objects])
AC_CONFIG_SRCDIR([src/ism/ism.cpp])
AC_CONFIG_SRCDIR([src/ism/purchaser/common.cpp])
AC_CONFIG_SRCDIR([src/helper/Helper.h])
AC_CONFIG_SRCDIR([src/helper/broker/ResourceBroker.cpp])
AC_CONFIG_SRCDIR([src/plan.h])

# Notices.
AC_COPYRIGHT([Copyright (c) 2004 The EU EGEE Project
See LICENCE file for details
])
AC_REVISION([$Revision: 1.22.2.3.2.2.2.1.2.1.4.5.2.8.2.1 $])

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
AC_CHECK_HEADERS([fcntl.h mntent.h sys/vfs.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_UID_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_HEADER_STDC
AC_FUNC_ERROR_AT_LINE
AC_FUNC_GETMNTENT
AC_FUNC_MEMCMP
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([bzero endpwent ftruncate getmntent memset mkdir pow strerror strtol])

AC_OPTIMIZE
GLITE_CHECK_LIBDIR
GLITE_CHECK_INITDIR
AC_GLITE_DOCBOOK

GLITE_CHECK_LDAP

PKG_CHECK_MODULES(ARES, libcares, , AC_MSG_ERROR(["ARES not found"]))
PKG_CHECK_MODULES(GSOAP, gsoapssl++, , AC_MSG_ERROR(["GSOAP not found"]))
AC_GSOAP_EXTRAS
PKG_CHECK_MODULES(GSOAP_PLUGIN, glite-security-gsoap-plugin++, , AC_MSG_ERROR(["GSOAP_PLUGIN not found"]))
AC_BOOST([], AC_MSG_RESULT(["BOOST ok"]), AC_MSG_ERROR(["BOOST not found"]))
PKG_CHECK_MODULES(CLASSAD, emi-classad-ns, , AC_MSG_ERROR(["CLASSADS not found"]))
#PKG_CHECK_MODULES(CPPUNIT, cppunit, , AC_MSG_ERROR(["CPPUNIT not found"]))
PKG_CHECK_MODULES(GLITE_WMSUTILS_EXCEPTION, jobman-exception, , AC_MSG_ERROR(["WMSUTILS_EXCEPTION not found"]))
PKG_CHECK_MODULES(GLITE_WMSUTILS_CLASSADS, classad-utils, , AC_MSG_ERROR(["WMSUTILS_CLASSADS not found"]))
PKG_CHECK_MODULES(GLITE_JDL, jdl-api-cpp, , AC_MSG_ERROR(["JDL not found"]))
PKG_CHECK_MODULES(GLITE_WMS_COMMON_CONF, wms-common-conf, , AC_MSG_ERROR(["WMS_COMMON_CONF not found"]))
PKG_CHECK_MODULES(GLITE_WMS_COMMON_LOGGER, wms-common-logger, , AC_MSG_ERROR(["WMS_COMMON_LOGGER not found"]))
PKG_CHECK_MODULES(GLITE_WMS_COMMON_PROCESS, wms-common-process, , AC_MSG_ERROR(["WMS_COMMON_PROCESS not found"]))
PKG_CHECK_MODULES(GLITE_WMS_COMMON_UT_UTIL, wms-common-util, , AC_MSG_ERROR(["WMS_COMMON_UT_UTIL not found"]))
PKG_CHECK_MODULES(GLITE_WMS_COMMON_UT_II, wms-common-ii, , AC_MSG_ERROR(["WMS_COMMON_UT_II not found"]))
PKG_CHECK_MODULES(GLITE_WMS_COMMON_QUOTA, wms-common-quota, , AC_MSG_ERROR(["WMS_COMMON_QUOTA not found"]))
PKG_CHECK_MODULES(GLITE_LB_COMMON, emi-lb-common, , AC_MSG_ERROR(["GLITE_LB_COMMON not found"]))
PKG_CHECK_MODULES(GLITE_LB_CLIENT, emi-lb-client, , AC_MSG_ERROR(["GLITE_LB_CLIENT not found"]))
PKG_CHECK_MODULES(GLITE_LB_CLIENTPP, emi-lb-clientpp, , AC_MSG_ERROR(["GLITE_LB_CLIENTPP not found"]))
PKG_CHECK_MODULES(RENEWAL, emi-proxyrenew, , AC_MSG_ERROR(["PROXY RENEWAL not found"]))
PKG_CHECK_MODULES(GLITE_WMS_PURGER, wms-purger, , AC_MSG_ERROR(["PURGER not found"]))

# Configuration items
AC_PREFIX_DEFAULT([/usr])
AM_CONFIG_HEADER([src/autogen/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([examples/Makefile])
AC_CONFIG_FILES([interface/Makefile])
AC_CONFIG_FILES([config/Makefile])
AC_CONFIG_FILES([config/glite-wms-wm])
AC_CONFIG_FILES([config/glite-wms-stats-setup.sh])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/ism/Makefile])
AC_CONFIG_FILES([src/ism/purchaser/Makefile])
AC_CONFIG_FILES([src/helper/Makefile])
AC_CONFIG_FILES([src/helper/broker/Makefile])
AC_CONFIG_FILES([src/helper/jobadapter/Makefile])

AC_OUTPUT
