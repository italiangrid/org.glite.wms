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
#       Revision 1.3  2004/07/25 18:25:37  eronchie
#       Start adding purger
#       WARNING: work in progress
#
#       Revision 1.2  2004/07/25 18:05:23  eronchie
#       Updated according to the new components
#
#       Revision 1.1.1.1  2004/07/18 12:14:57  eronchie
#       Start adding purger component
#
#       Revision 1.7  2004/06/21 12:56:33  eronchie
#       Start handling of interface headers
#
#       Revision 1.6  2004/06/16 15:57:56  eronchie
#       Added configure options
#       WARNING: work in progress
#
#       Revision 1.5  2004/06/16 08:24:07  eronchie
#       Added configure options
#       WARNING: work in progress
#
#       Revision 1.4  2004/06/15 17:35:14  eronchie
#       Added configure options
#       WARNING: work in progress
#
#       Revision 1.3  2004/06/14 17:50:09  eronchie
#       Added configure options
#       WARNING Work in progress
#
#       Revision 1.2  2004/05/31 13:39:51  eronchie
#       Merged with code in the infnforge repository
#
#       Revision 1.1.1.2  2004/05/26 18:24:28  eronchie
#       Import of wms tls
#
#
                                                                                                                                                             
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([GLite WMS Tls], [0.0.0])
AC_CONFIG_AUX_DIR([./project])
AM_INIT_AUTOMAKE([1.6.3 subdir-objects])
AC_CONFIG_SRCDIR([src/purger.cpp])

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

AC_BOOST([], have_boost=yes, have_boost=no)
AC_CLASSADS([], have_classads=yes, have_classads=no)
AC_GLOBUS([], have_globus=yes, have_globus=no)

AC_MSG_RESULT(["CLASSADS found $have_classads"])
AC_MSG_RESULT(["BOOST found $have_boost"])
AC_MSG_RESULT(["GLOBUS found $have_globus"])

have_glite_wms_common=no
have_glite_wmsutils_jobid=no
have_glite_wmsutils_exception=no
have_glite_lb=no

AC_GLITE

AC_GLITE_WMS_COMMON([], have_glite_wms_common=yes, have_glite_wms_common=no)
AC_MSG_RESULT(["GLITE_WMS_COMMON found $have_glite_wms_common"])

AC_GLITE_WMSUTILS_JOBID([], have_glite_wmsutils_jobid=yes, have_glite_wmsutils_jobid=no)
AC_MSG_RESULT(["GLITE_WMSUTILS_JOBID found $have_glite_wmsutils_jobid"])

AC_GLITE_WMSUTILS_EXCEPTION([], have_glite_wmsutils_exception=yes, have_glite_wmsutils_exception=no)
AC_MSG_RESULT(["GLITE_WMSUTILS_EXCEPTION found $have_glite_wmsutils_exception"])

AC_GLITE_LB([], have_glite_lb=yes, have_glite_lb=no)
AC_MSG_RESULT(["GLITE_LB found $gave_glite_lb"])

# Configuration items
AC_PREFIX_DEFAULT([/opt/glite])
AM_CONFIG_HEADER([src/autogen/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([interface/Makefile])
AC_CONFIG_FILES([src/Makefile])

AC_OUTPUT
