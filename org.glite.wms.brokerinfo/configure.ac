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
#       Revision 1.14  2005/06/10 14:26:23  monforte
#       Merge RC1 -> HEAD
#
#       Revision 1.10.2.12  2005/05/09 16:49:53  emartel
#       increased version
#
#       Revision 1.10.2.11  2005/05/09 16:23:38  emartel
#       Added ares dependency
#
#       Revision 1.10.2.10  2005/04/11 12:13:22  eronchie
#       Added AC_OPTIMIZE
#       Added --disable-optimization
#       Added optimize.m4
#
#       Revision 1.10.2.9  2005/03/24 11:20:47  emartel
#       increased version
#
#       Revision 1.10.2.8  2005/03/23 10:07:45  emartel
#       Increased version due to the addition of secure access to StrorageIndex Catalog
#
#       Revision 1.10.2.7  2005/02/17 18:12:50  emartel
#       increased version
#
#       Revision 1.10.2.6  2005/02/17 18:00:43  emartel
#       Added control on soap.fault in gsoap request towards DLI and StorageIndex Catalogs
#
#       Revision 1.10.2.5  2005/02/15 14:12:43  monforte
#       Corrected version numbers
#
#       Revision 1.10.2.4  2005/02/15 12:10:30  monforte
#       Added CE-SE binding resolution from ISM
#
#       Revision 1.10.2.3  2005/02/09 14:55:55  emartel
#       increased version
#
#       Revision 1.10.2.2  2005/02/09 14:04:24  emartel
#       increased version
#
#       Revision 1.10.2.1  2005/01/12 07:54:02  eronchie
#       Updated
#
#       Revision 1.10  2004/12/10 07:43:36  eronchie
#       Increased version
#
#       Revision 1.9  2004/12/09 11:30:55  eronchie
#       Updated
#
#       Revision 1.8  2004/12/09 11:18:13  eronchie
#       Updated data info
#
#       Revision 1.7  2004/12/09 10:32:04  eronchie
#       Updated dependency
#
#       Revision 1.6  2004/12/07 18:47:45  eronchie
#       Work In progress
#
#       Revision 1.5  2004/12/02 16:45:53  emartel
#       Added Storage Index catalog interface and Data Location catalog interface
#
#       Revision 1.4  2004/10/08 15:03:26  monforte
#       rls integration
#
#       Revision 1.3  2004/09/22 00:59:10  dimeglio
#       Fixed missing parenthesis in AC_INIT
#
#       Revision 1.2  2004/09/21 19:24:20  glbuild
#       modified module.version
#
#       Revision 1.1  2004/06/28 22:58:08  eronchie
#       Start adding brokerinfo
#       WARNING: work in progress
#
#       Revision 1.1  2004/06/26 15:25:03  eronchie
#       Start handling matchmaking
#       WARNING: work in progress
#
#
#
#
                                                                                                                                                             
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([GLite WMS Brokerinfo], [1.2.4])
AC_CONFIG_AUX_DIR([./project])
AM_INIT_AUTOMAKE([1.6.3 subdir-objects])
AC_CONFIG_SRCDIR([src/glue_attributes.h])

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
have_ares=no
have_cgsi_gsoap=no
have_replica=no
have_data=no

AC_BOOST([], have_boost=yes, have_boost=no)
AC_CLASSADS([], have_classads=yes, have_classads=no)
AC_GLOBUS([], have_globus=yes, have_globus=no)
AC_ARES([], have_ares=yes, have_ares=no)
AC_GSOAP
AC_CGSI_GSOAP
AC_REPLICA([], have_replica=yes, have_replica=no)

AC_MSG_RESULT(["CLASSADS found $have_classads"])
AC_MSG_RESULT(["BOOST found $have_boost"])
AC_MSG_RESULT(["GLOBUS found $have_globus"])
AC_MSG_RESULT(["REPLICA found $have_replica"])

AC_OPTIMIZE
AC_GLITE

AC_SEC_GSOAP_PLUGIN

AC_GLITE_DATA([], have_data=yes, have_data=no)
AC_MSG_RESULT(["GLITE_DATA found $have_data"])

# Configuration items
AC_PREFIX_DEFAULT([/opt/glite])
AM_CONFIG_HEADER([src/autogen/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([interface/Makefile])
AC_CONFIG_FILES([src/Makefile])

AC_OUTPUT

