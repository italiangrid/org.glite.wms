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
#       Revision 1.3  2004/06/01 13:52:35  eronchie
#       Updated namespace for jobid.
#       Changed glite::wms::common::jobid namespace in glite::wms::jobid
#
#       Revision 1.2  2004/05/31 14:03:30  eronchie
#       Merged with code in the infnforge repository
#
#       Revision 1.1.1.1  2004/05/26 18:45:05  eronchie
#       Import of wms jobid
#
#
#
                                                                                                                                                             
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([GLite WMS jobid], [0.0.0])
AC_CONFIG_AUX_DIR([./project])
AM_INIT_AUTOMAKE([1.6.3 subdir-objects])
AC_CONFIG_SRCDIR([src/jobid/cjobid.h])

# Notices.
AC_COPYRIGHT([Copyright (c) 2004 The EU EGEE Project
See LICENCE file for details
])
AC_REVISION([$Revision$])

#Environment.
WORKDIR=`pwd`
AC_SUBST(WORKDIR)

REPOSITORY=$WORKDIR/../../repository
STAGE=$WORKDIR/../../stage

DISTBIN=$WORKDIR/../../dist
AC_SUBST(DISTBIN)

BOOST_CFLAGS=-I$REPOSITORY/boost/1.29.1/rhel30_gcc32/include
AC_SUBST([BOOST_CFLAGS])
BOOST_CPPFLAGS=$BOOST_CFLAGS
AC_SUBST([BOOST_CPPFLAGS])
BOOST_LIBS="-L$REPOSITORY/boost/1.29.1/rhel30_gcc32/lib/release -lboost_fs -lboost_thread -lpthread -lboost_regex -lc"
AC_SUBST([BOOST_LIBS])

CLASSAD_CFLAGS=-I$REPOSITORY/classads/0.9.5/rhel30_gcc32/include
AC_SUBST([CLASSAD_CFLAGS])
CLASSAD_CPPFLAGS=$CLASSAD_CFLAGS
AC_SUBST([CLASSAD_CPPFLAGS])
CLASSAD_LIBS="-L$REPOSITORY/classads/0.9.5/rhel30_gcc32/lib -lclassad"
AC_SUBST([CLASSAD_LIBS])

GLOBUS_SSL_UTILS_CFLAGS="-I$STAGE/include"
AC_SUBST([GLOBUS_SSL_UTILS_CFLAGS])
#GLOBUS_SSL_UTILS_LIBS="-L$STAGE/lib -lglobus_ssl_utils"
#AC_SUBST([GLOBUS_SSL_UTILS_LIBS])

GLITE_WORKLOAD_CFLAGS="-I$STAGE/include"
AC_SUBST([GLITE_WORKLOAD_CFLAGS])
GLITE_WORKLOAD_COMMON_LIBS="-L$STAGE/lib -lglite_wms_exceptions"
AC_SUBST([GLITE_WORKLOAD_COMMON_LIBS])

GLOBUS_COMMON_NOTHR_LIBS="-L$REPOSITORY/globus/2.4.3/rhel30_gcc32/lib -lglobus_common_gcc32"
AC_SUBST([GLOBUS_COMMON_NOTHR_LIBS])
GLOBUS_COMMON_THR_LIBS="-L$REPOSITORY/globus/2.4.3/rhel30_gcc32/lib -lglobus_common_gcc32pthr"
AC_SUBST([GLOBUS_COMMON_THR_LIBS])

GLOBUS_FTP_CLIENT_NOTHR_LIBS="-L$REPOSITORY/globus/2.4.3/rhel30_gcc32/lib -lglobus_ftp_client_gcc32"
AC_SUBST([GLOBUS_FTP_CLIENT_NOTHR_LIBS])
GLOBUS_FTP_CLIENT_THR_LIBS="-L$REPOSITORY/globus/2.4.3/rhel30_gcc32/lib -lglobus_ftp_client_gcc32pthr"
AC_SUBST([GLOBUS_FTP_CLIENT_THR_LIBS])
 
GLOBUS_SSL_NOTHR_LIBS="-L$REPOSITORY/globus/2.4.3/rhel30_gcc32/lib -lssl_gcc32 -lcrypto_gcc32"
AC_SUBST([GLOBUS_SSL_NOTHR_LIBS])
GLOBUS_SSL_THR_LIBS="-L$REPOSITORY/globus/2.4.3/rhel30_gcc32/lib -lssl_gcc32pthr -lcrypto_gcc32pthr"
AC_SUBST([GLOBUS_SSL_THR_LIBS])

GLOBUS_CFLAGS=-I$REPOSITORY/globus/2.4.3/rhel30_gcc32/include/gcc32
AC_SUBST([GLOBUS_CFLAGS])
GLOBUS_CPPFLAGS=$GLOBUS_CFLAGS
AC_SUBST([GLOBUS_CPPFLAGS])
GLOBUS_LIBS="-L$REPOSITORY/globus/2.4.3/rhel30_gcc32/lib -lglobus_gass_copy_gcc32 -lglobus_ftp_client_gcc32 -lglobus_gass_transfer_gcc32 -lglobus_ftp_control_gcc32 -lglobus_io_gcc32 -lglobus_gss_assist_gcc32 -lglobus_gssapi_gsi_gcc32 -lglobus_gsi_proxy_core_gcc32 -lglobus_gsi_credential_gcc32 -lglobus_gsi_callback_gcc32 -lglobus_oldgaa_gcc32 -lglobus_gsi_sysconfig_gcc32 -lglobus_gsi_cert_utils_gcc32 -lglobus_openssl_error_gcc32 -lglobus_openssl_gcc32 -lglobus_proxy_ssl_gcc32 -lssl_gcc32 -lcrypto_gcc32 -lglobus_common_gcc32"
AC_SUBST([GLOBUS_LIBS])

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

# Configuration items
AC_PREFIX_DEFAULT([/opt/glite])
AM_CONFIG_HEADER([src/autogen/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/jobid/Makefile])
AC_CONFIG_FILES([test/Makefile])

AC_OUTPUT
