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
#       Authors: Giuseppe Avellino <giuseppe.avellino@datamat.it>
#

AC_PREREQ(2.57)
AC_INIT([GLite WMS wmproxy], [0.1.0])
AC_CONFIG_AUX_DIR([./project])
AM_INIT_AUTOMAKE([1.6.3 subdir-objects])
AC_CONFIG_SRCDIR([src/server/wmproxy.h])

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
AC_C_CONST
AC_C_INLINE
AC_HEADER_TIME

# Checks for library functions.
AC_HEADER_STDC
AC_CHECK_FUNCS([memset])

have_boost=no
have_classads=no                                                                
have_globus=no
have_ares=no
have_expat=no
have_fcgi=no
have_xml2=no
 
AC_BOOST([], have_boost=yes, have_boost=no)
AC_MSG_RESULT(["BOOST found $have_boost"])

AC_CLASSADS([], have_classads=yes, have_classads=no)
AC_MSG_RESULT(["CLASSADS found $have_classads"])

AC_GLOBUS([], have_globus=yes, have_globus=no)
AC_MSG_RESULT(["GLOBUS found $have_globus"])

AC_ARES([], have_ares=yes, have_ares=no)
AC_MSG_RESULT(["ARES found $have_ares"])

AC_EXPAT([], have_expat=yes, have_expat=no)
AC_MSG_RESULT(["EXPAT found $have_expat"])

AC_FCGI([], have_fcgi=yes, have_fcgi=no)
AC_MSG_RESULT(["FCGI found $have_fcgi"])

AC_XML2([], have_xml2=yes, have_xml2=no)
AC_MSG_RESULT(["XML2 found $have_xml2"])

have_glite_wmsutils=no
                                                                                
AC_GLITE

AC_GSOAP

AC_GLITE_WMSUTILS([], have_glite_wmsutils=yes, have_glite_wmsutils=no)
AC_MSG_RESULT(["GLITE_WMSUTILS found $have_glite_wmsutils"])

# Configuration items
AC_PREFIX_DEFAULT([/opt/glite])
AM_CONFIG_HEADER([src/autogen/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/server/Makefile])
#AC_CONFIG_FILES([test/Makefile])
#AC_CONFIG_FILES([interface/Makefile])
AC_CONFIG_FILES([wmproxy.dox])

AC_OUTPUT


