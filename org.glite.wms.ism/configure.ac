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
#       Revision 1.9  2005/02/28 10:50:17  monforte
#       *** empty log message ***
#
#       Revision 1.7.2.8  2005/02/15 12:18:14  monforte
#       Fixed CE SE binding
#
#       Revision 1.7.2.7  2005/02/08 21:11:50  monforte
#       increased version nummber
#
#       Revision 1.7.2.6  2005/02/03 14:34:58  monforte
#       Added and improved some test.
#       Added handling of balck list in each purchaser
#
#       Revision 1.7.2.5  2005/01/26 14:52:17  eronchie
#       Updated
#       Added AC_GLITE_CE
#
#       Revision 1.7.2.4  2005/01/20 15:45:34  eronchie
#       Updated
#       fix bug for expiry ism entries
#
#       Revision 1.7.2.3  2005/01/12 10:59:52  eronchie
#       Updated
#
#       Revision 1.7.2.2  2004/12/27 11:21:31  eronchie
#       Updated
#
#       Revision 1.7.2.1  2004/12/20 08:25:55  dimeglio
#       Incremented patch number
#
#       Revision 1.7  2004/12/10 07:48:44  eronchie
#       Increased version
#
#       Revision 1.6  2004/11/25 16:32:30  monforte
#       updating verison numbers
#
#       Revision 1.5  2004/11/10 11:00:09  monforte
#       *** empty log message ***
#
#       Revision 1.4  2004/11/09 10:14:39  monforte
#       Added first experimental prototype of ism purchaser for cemonitor...
#
#       Revision 1.3  2004/09/22 01:02:25  dimeglio
#       Fixed missing parenthesis in AC_INIT
#
#       Revision 1.2  2004/09/21 12:48:58  eronchie
#       Updated
#
#       Revision 1.1.1.1  2004/09/21 12:02:49  eronchie
#       Moved ism from manager component into the new component ism
#
#
                                                                                                                                                             
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([GLite WMS ISM], [1.0.4])
AC_CONFIG_AUX_DIR([./project])
AM_INIT_AUTOMAKE([1.6.3 subdir-objects])
AC_CONFIG_SRCDIR([src/ism.cpp])

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
have_cppunit=no
have_globus=no

AC_BOOST([], have_boost=yes, have_boost=no)
AC_CLASSADS([], have_classads=yes, have_classads=no)
AC_CPPUNIT([], have_cppunit=yes, have_cppunit=no)
AC_GLOBUS([], have_globus=yes, have_globus=no)

AC_MSG_RESULT(["CLASSADS found $have_classads"])
AC_MSG_RESULT(["BOOST found $have_boost"])
AC_MSG_RESULT(["CPPUNIT found $have_cppunit"])
AC_MSG_RESULT(["GLOBUS found $have_globus"])

have_glite_wms_common=no
have_glite_wms_jdl=no
have_glite_wms_ism=no
have_glite_wmsutils=no
have_glite_ce=no

AC_GLITE

AC_GSOAP

AC_OPTIMIZE

AC_GLITE_WMS_COMMON([], have_glite_wms_common=yes, have_glite_wms_common=no)
AC_GLITE_WMSUTILS([], have_glite_wmsutils=yes, have_glite_wmsutils=no)

AC_GLITE_WMS_JDL([], have_glite_wms_jdl=yes, have_glite_wms_jdl=no)
AC_MSG_RESULT(["GLITE_WMS_JDL found $have_glite_wms_jdl"])

AC_GLITE_WMS_ISM([], have_glite_wms_ism=yes, have_glite_wms_ism=no)
AC_MSG_RESULT(["GLITE_WMS_ISM found $have_glite_wms_ism"])

AC_GLITE_WMSUTILS([], have_glite_wmsutils=yes, have_glite_wmsutils=no)
AC_MSG_RESULT(["GLITE_WMSUTILS found $have_glite_wmsutils"])

AC_GLITE_CE([], have_glite_ce=yes, have_glite_ce=no)
AC_MSG_RESULT(["GLITE_CE found $have_glite_ce"])

# Configuration items
AC_PREFIX_DEFAULT([/opt/glite])
AM_CONFIG_HEADER([src/autogen/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([example/Makefile])
AC_CONFIG_FILES([interface/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/purchaser/Makefile])
AC_CONFIG_FILES([test/Makefile])

AC_OUTPUT
