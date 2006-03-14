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
#       Common configure.ac file for the GLite WMS Common module
#
#       Authors: Alberto Di Meglio <alberto.di.meglio@cern.ch>
#       Version info: $Id$
#       Release: $Name$
#
#       Revision history:
#       $Log$
#       Revision 1.17  2006/03/14 15:51:36  eronchie
#       Applied Purer macro
#
#       Revision 1.16  2005/10/15 12:04:48  eronchie
#       Increased version
#
#       Revision 1.15  2005/10/14 17:14:50  emartel
#       added service discovery
#
#       Revision 1.14  2005/10/04 11:39:44  eronchie
#       Applied changes due to rgma api
#
#       Revision 1.13  2004/12/09 17:33:28  giaco
#       set version number to 1.0.0
#
#       Revision 1.12  2004/12/04 21:20:23  dimeglio
#       Increased version number
#
#       Revision 1.11  2004/11/01 10:41:51  dimeglio
#       Incremented version number
#
#       Revision 1.10  2004/10/22 12:24:34  eronchie
#       Added optimize support
#
#       Revision 1.9  2004/10/14 14:15:22  eronchie
#       Updated
#
#       Revision 1.8  2004/10/13 22:02:23  eronchie
#       Added ism dependency
#
#       Revision 1.7  2004/10/01 13:52:12  pandreet
#       Build files for unit tests
#
#       Revision 1.6  2004/09/22 01:04:44  dimeglio
#       Fixed missing parenthesis in AC_INIT
#
#       Revision 1.5  2004/09/21 19:25:54  glbuild
#       modified module.version
#
#       Revision 1.4  2004/08/27 13:34:32  eronchie
#       Added handling of config dir
#       Added start up script file
#
#       Revision 1.3  2004/08/09 07:45:06  eronchie
#       Updated
#       WARNING: work in progress
#
#       Revision 1.2  2004/08/09 06:35:48  eronchie
#       Added check macros
#
#       Revision 1.1.1.1  2004/08/08 17:03:21  eronchie
#       Start adding manager component
#
#
#
#
                                                                                                                                                             
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([GLite WMS Manager], [1.5.0])
AC_CONFIG_AUX_DIR([./project])
AM_INIT_AUTOMAKE([1.6.3 subdir-objects])
AC_CONFIG_SRCDIR([src/common/WMImpl.h])

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

have_boost=no
have_classads=no
have_globus=no
have_ares=no
have_expat=no
have_cppunit=no

AC_BOOST([], have_boost=yes, have_boost=no)
AC_CLASSADS([], have_classads=yes, have_classads=no)
AC_GLOBUS([], have_globus=yes, have_globus=no)
AC_ARES([], have_ares=yes, have_ares=no)
AC_EXPAT([], have_expat=yes, have_expat=no)
AC_CPPUNIT([], have_cppunit=yes, have_cppunit=no)
                                                                               
AC_MSG_RESULT(["CLASSADS found $have_classads"])
AC_MSG_RESULT(["BOOST found $have_boost"])
AC_MSG_RESULT(["GLOBUS found $have_globus"])
AC_MSG_RESULT(["ARES found $have_ares"])
AC_MSG_RESULT(["EXPAT found $have_expat"])
AC_MSG_RESULT(["CPPUNIT found $have_cppunit"])

AM_PATH_LOG4CXX

have_glite_wms_common=no
have_glite_wmsutils=no
have_glite_wms_jdl=no
have_glite_wms_helper=no
have_glite_wms_jss=no
have_glite_lb=no
have_glite_security=no
have_glite_wms_ism=no
have_glite_wms_purger=no

AC_GLITE
AC_GLITE_SERVICE_DISCOVERY

AC_GLITE_WMS_COMMON([], have_glite_wms_common=yes, have_glite_wms_common=no)
AC_MSG_RESULT(["GLITE_WMS_COMMON found $have_glite_wms_common"])

AC_GLITE_WMSUTILS([], have_glite_wmsutils=yes, have_glite_wmsutils=no)
AC_MSG_RESULT(["GLITE_WMSUTILS found $have_glite_wmsutils"])

AC_GLITE_WMS_HELPER([], have_glite_wms_helper=yes, have_glite_wms_helper=no)
AC_MSG_RESULT(["GLITE_WMS_HELPER found $have_glite_wms_helper"])

AC_GLITE_WMS_JSS([], have_glite_wms_jss=yes, have_glite_wms_jss=no)
AC_MSG_RESULT(["GLITE_WMS_JSS found $have_glite_wms_jss"])

AC_GLITE_WMS_JDL([], have_glite_wms_jdl=yes, have_glite_wms_jdl=no)
AC_MSG_RESULT(["GLITE_WMS_JDL found $have_glite_wms_jdl"])

AC_GLITE_WMS_PURGER([], have_glite_wms_purger=yes, have_glite_wms_purger=no)
AC_MSG_RESULT(["GLITE_WMS_PURGER found $have_glite_wms_purger"])

AC_GLITE_WMS_ISM([], have_glite_wms_ism=yes, have_glite_wms_ism=no)
AC_MSG_RESULT(["GLITE_WMS_ISM found $have_glite_wms_ism"])

AC_GLITE_LB([], have_glite_lb=yes, have_glite_lb=no)
AC_MSG_RESULT(["GLITE_LB found $have_glite_lb"])

AC_GLITE_SECURITY([], have_glite_security=yes, have_glite_security=no)
AC_MSG_RESULT(["GLITE_SECURITY found $have_glite_security"])

# Configuration items
AC_PREFIX_DEFAULT([/opt/glite])
AM_CONFIG_HEADER([src/autogen/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([config/Makefile])
AC_CONFIG_FILES([config/glite-wms-wm])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/common/Makefile])
AC_CONFIG_FILES([src/daemons/Makefile])
AC_CONFIG_FILES([src/server/Makefile])
AC_CONFIG_FILES([src/server/glite-wms-dag-post.sh])
AC_CONFIG_FILES([src/server/glite-wms-planner.sh])
AC_CONFIG_FILES([test/Makefile])

AC_OUTPUT

