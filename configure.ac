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
#       Authors: Paolo Andreetto <paolo.andreetto@pd.infn.it>
#       Version info: $Id$
#       Release: $Name$
#
#       Revision history:
#       $Log$
#       Revision 1.3  2004/07/18 11:56:40  eronchie
#       Updated LB dependency
#
#       Revision 1.2  2004/07/18 11:53:35  eronchie
#       Added AC_GLITE_LB check
#       WARNING: work in progress
#
#       Revision 1.1  2004/07/06 10:05:38  pandreet
#       Basic build files for jobsubmission
#
#
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([GLite WMS jobsubmission], [0.0.0])
AC_CONFIG_AUX_DIR([./project])
AM_INIT_AUTOMAKE([1.6.3 subdir-objects])
AC_CONFIG_SRCDIR([src/common/constants.h])

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
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MKTIME
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([ftruncate localtime_r pow strdup strerror strstr])

have_boost=no
have_classads=no
have_condorg=no
have_globus=no
have_globus_ssl=no
                                                                                
AC_BOOST([], have_boost=yes, have_boost=no)
AC_CLASSADS([], have_classads=yes, have_classads=no)
AC_CONDORG(6.5.3, have_condorg=yes, have_condorg=no)
AC_GLOBUS([], have_globus=yes, have_globus=no)

AC_GLOBUS_SSL_UTILS([], have_globus_ssl=yes, have_globus_ssl=no)
                                                                                
AC_MSG_RESULT(["CLASSADS found $have_classads"])
AC_MSG_RESULT(["BOOST found $have_boost"])
AC_MSG_RESULT(["CONDORG found $have_condorg"])
AC_MSG_RESULT(["GLOBUS found $have_globus"])
AC_MSG_RESULT(["GLOBUS_SSL found $have_globus_ssl"])

have_glite_wms_common=no
have_glite_wms_jobid=no
have_glite_wms_tls=no
have_glite_wms_jdl=no
have_glite_lb=no                                                          
                      
AC_GLITE
                                                                                
AC_GLITE_WMS_COMMON([], have_glite_wms_common=yes, have_glite_wms_common=no)
AC_MSG_RESULT(["GLITE_WMS_COMMON found $have_glite_wms_common"])

AC_GLITE_WMS_JOBID([], have_glite_wms_jobid=yes, have_glite_wms_jobid=no)
AC_MSG_RESULT(["GLITE_WMS_JOBID found $have_glite_wms_jobid"])

AC_GLITE_WMS_TLS([], have_glite_wms_tls=yes, have_glite_wms_tls=no)
AC_MSG_RESULT(["GLITE_WMS_TLS found $have_glite_wms_tls"])

AC_GLITE_WMS_JDL([], have_glite_wms_jdl=yes, have_glite_wms_jdl=no)
AC_MSG_RESULT(["GLITE_WMS_JDL found $have_glite_wms_jdl"])

AC_GLITE_LB([], have_glite_lb=yes, have_glite_lb=no)
AC_MSG_RESULT(["GLITE_LB found $have_glite_lb"])

# Configuration items
AC_PREFIX_DEFAULT([/opt/glite])
AM_CONFIG_HEADER([src/autogen/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/common/Makefile])
AC_CONFIG_FILES([src/controller/Makefile])
AC_CONFIG_FILES([src/logmonitor/Makefile])
AC_CONFIG_FILES([src/logmonitor/processer/Makefile])
AC_CONFIG_FILES([src/daemons/Makefile])
AC_CONFIG_FILES([test/Makefile])

AC_OUTPUT
