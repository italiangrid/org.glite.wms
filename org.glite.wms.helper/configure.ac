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
#       Revision 1.24  2006/11/23 10:08:00  monforte
#       Removed ism/broker/brokerinfo/matchmaking dependencies
#
#       Revision 1.23  2006/03/21 16:10:05  eronchie
#       Added macro
#
#       Revision 1.22  2006/03/20 22:08:54  eronchie
#       Increased version
#
#       Revision 1.21  2006/03/17 07:51:00  eronchie
#       Updated
#       Applied changed due to the new locations of jdl and classad_utils
#
#       Revision 1.20  2006/02/09 14:15:49  mcecchi
#       URL moved
#
#       Revision 1.19  2006/01/24 12:16:03  mcecchi
#       jobwrapper unification
#
#       Revision 1.18  2005/10/15 11:57:04  eronchie
#       Increased version
#
#       Revision 1.17  2005/10/14 09:12:08  eronchie
#       Added security and gbox macros
#
#       Revision 1.16  2005/10/07 15:45:20  eronchie
#       Added templates handling
#
#       Revision 1.15  2005/09/22 11:42:50  eronchie
#       Started merging with rc1.4
#
#       Revision 1.14  2005/06/13 08:18:08  monforte
#       Merge RC1 -> HEAD
#
#       Revision 1.12.2.9  2005/06/08 15:01:04  eronchie
#       Added (
#       Increased version
#
#       Revision 1.12.2.8  2005/04/17 18:33:29  eronchie
#       Removed temporarely test/Makefile
#
#       Revision 1.12.2.7  2005/04/15 16:18:27  vmedici
#       unit test
#
#       Revision 1.12.2.6  2005/04/06 08:38:18  fprelz
#       Increased revision number, as already done by 'dimeglio' on
#       org.glite.wms.helper/project/version.properties
#
#       Revision 1.12.2.5  2005/03/14 10:45:23  eronchie
#       Increased version
#
#       Revision 1.12.2.4  2005/02/18 14:35:41  eronchie
#       Updated
#       Increased version for the following reason:
#
#       "Modified JobAdapter to allow submission of Condor 'vanilla' jobs
#       via Condor-C"
#
#       Revision 1.12.2.3  2005/02/08 15:17:26  fprelz
#       Increased minor version, as the broker helper links in the matchmaking
#       library statically, and the latter received a bug fix.
#
#       Revision 1.12.2.2  2005/01/17 16:54:38  eronchie
#       Updated optimization
#
#       Revision 1.12.2.1  2005/01/13 17:01:41  eronchie
#       Added patch related to bug 3931
#
#       Revision 1.12  2004/12/10 07:47:50  eronchie
#       Increased version
#
#       Revision 1.11  2004/11/26 08:03:03  eronchie
#       Increased version
#
#       Revision 1.10  2004/10/13 21:43:24  eronchie
#       Added libraries
#
#       Revision 1.9  2004/09/22 01:01:27  dimeglio
#       Fixed missing parenthesis in AC_INIT
#
#       Revision 1.8  2004/09/21 19:24:45  glbuild
#       modified module.version
#
#       Revision 1.7  2004/07/28 10:51:21  eronchie
#       Added m4files.list property
#
#       Revision 1.6  2004/07/27 11:34:57  eronchie
#       Start using AC_GLITE_WMSUTILS
#
#       Revision 1.5  2004/07/23 14:13:18  eronchie
#       jobid was moved under wmsutils subsystem
#
#       Revision 1.4  2004/07/06 13:55:36  monforte
#       Initial Helpers porting to new build system
#       WARNING: work in progress...
#
#       Revision 1.3  2004/07/05 16:06:00  eronchie
#       Start handling helper
#       WARNING: work in progress
#
#       Revision 1.2  2004/07/05 14:55:56  eronchie
#       Start handling helper
#       WARNING: work in progress
#
#       Revision 1.1  2004/07/02 13:11:39  monforte
#       Initial Helper porting to new build system
#       WARNING: work in progress
#
#       Revision 1.2  2004/06/30 08:35:15  monforte
#       Initial MatchMaking porting to new build system
#
#       Revision 1.1  2004/06/26 15:25:03  eronchie
#       Start handling matchmaking
#       WARNING: work in progress
#
#
#
                                                                                                                                                             
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([GLite WMS Helper], [3.1.0])
AC_CONFIG_AUX_DIR([./project])
AM_INIT_AUTOMAKE([1.6.3 subdir-objects])
AC_CONFIG_SRCDIR([src/Helper.h])

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

have_boost=no
have_classads=no
have_globus=no
have_cppunit=no

AC_BOOST([], have_boost=yes, have_boost=no)
AC_MSG_RESULT(["BOOST found $have_boost"])

AC_CLASSADS([], have_classads=yes, have_classads=no)
AC_MSG_RESULT(["CLASSADS found $have_classads"])

AC_GLOBUS([], have_globus=yes, have_globus=no)
AC_MSG_RESULT(["GLOBUS found $have_globus"])

AC_CPPUNIT(1.10.2,have_cppunit=yes, have_cppunit=no)
AC_MSG_RESULT(["CPPUNIT found $have_cppunit"])

if test "$have_globus" = "no"; then
  AC_MSG_RESULT(["Please verify GLOBUS package"])
fi

if test "$have_boost" = "no"; then
  AC_MSG_RESULT(["Please verify BOOST package"])
fi

if test "$have_classads" = "no"; then
  AC_MSG_RESULT(["Please verify CLASSADS package"])
fi

if test "$have_cppunit" = "no"; then
  AC_MSG_RESULT(["Please verify CPPUNIT package"])
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

if test "$have_cppunit" = "no"; then
  AC_MSG_ERROR([Cannot continue building])
fi

have_glite_wmsutils=no
have_glite_wms_common=no
have_glite_wms_jdl=no

AC_GLITE
AC_OPTIMIZE
AC_GLITE_WMS_OPTIONAL

AC_GLITE_WMS_COMMON([], have_glite_wms_common=yes, have_glite_wms_common=no)
AC_GLITE_JDL([], have_glite_jdl=yes, have_glite_jdl=no)

AC_GLITE_WMSUTILS([], have_glite_wmsutils=yes, have_glite_wmsutils=no)

AC_MSG_RESULT(["GLITE_WMS_COMMON found $have_glite_wms_common"])
AC_MSG_RESULT(["GLITE_JDL found $have_glite_jdl"])
AC_MSG_RESULT(["GLITE_WMSUTILS found $have_glite_wmsutils"])

have_security=no
have_gpbox=no
AC_GLITE_SECURITY([], have_security=yes, have_security=no)
AC_MSG_RESULT(["GLITE_GLITE_SECURITY found $have_security"])

AC_GLITE_GPBOX([], have_gpbox=yes, have_gpbox=no)
AC_MSG_RESULT(["GLITE_GLITE_GPBOX found $have_gpbox"])

# Configuration items
AC_PREFIX_DEFAULT([/opt/glite])
AM_CONFIG_HEADER([src/autogen/config.h])
AC_CONFIG_FILES([config/Makefile])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([examples/Makefile])
# AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([interface/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/jobadapter/Makefile])

AC_OUTPUT

