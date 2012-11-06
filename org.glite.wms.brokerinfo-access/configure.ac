#
# Copyright (c) 2004 on behalf of the EU EGEE Project:
# The European Organization for Nuclear Research (CERN),
# Istituto Nazionale di Fisica Nucleare (INFN), Italy
# Datamat Spa, Italy
# Centre National de la Recherche Scientifique (CNRS), France
# CS Systeme d'Information (CSSI), France
# Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
# Universiteit van Amsterdam (UvA), Netherlands
# University of Helsinki (UH.HIP), Finland
# University of Bergen (UiB), Norway
# Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
#

AC_PREREQ(2.57)
AC_INIT([GLite WMS Brokerinfo Access], [3.3.2])
AC_CONFIG_AUX_DIR([./project])
AM_INIT_AUTOMAKE([1.6.3 subdir-objects])
AC_CONFIG_SRCDIR([src/BrokerInfo.C])

# Notices.
AC_COPYRIGHT([Copyright (c) 2004 The EU EGEE Project
See LICENCE file for details
])
AC_REVISION([$Revision: 1.5.6.2.2.4 $])

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

GLITE_CHECK_LIBDIR
AC_OPTIMIZE
AC_GLITE_DOCBOOK

PKG_CHECK_MODULES(CLASSAD, emi-classad-ns, , AC_MSG_ERROR(["CLASSADS not found"]))

DX_DOXYGEN_FEATURE(ON)
DX_DOT_FEATURE(OFF)
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(glite-wms-brokerinfo-access, project/doxygen/doxygen.cfg, autodoc)

# Configuration items
AC_PREFIX_DEFAULT([/usr])
AM_CONFIG_HEADER([src/autogen/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/README])
AC_CONFIG_FILES([src/brokerinfo-access.pc])
AC_CONFIG_FILES([doc/Makefile])

AC_OUTPUT
