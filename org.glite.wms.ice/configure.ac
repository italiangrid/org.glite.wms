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
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([GLite WMS ICE], [3.1.0])
AC_CONFIG_AUX_DIR([./project])
AM_INIT_AUTOMAKE([1.6.3 subdir-objects])
AC_CONFIG_SRCDIR([src/ice/ice-core.h])

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
AM_PATH_LOG4CPP

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([string])
AC_CHECK_HEADERS([iostream])
AC_CHECK_HEADERS([time.h])
AC_CHECK_HEADERS([string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_HEADER_TIME

# Checks for library functions.
AC_HEADER_STDC
AC_CHECK_FUNCS([memset])

#
# The following block is a TEMPORARY FIX to build problems on SLC4.
# This block should be removed and placed in an appropriate file in 
# org.glite.build.common-cpp
#
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
old_LIBS=$LIBS
bdb_lib_list="db_cxx db_cxx-4.2"
for bdb_lib in $bdb_lib_list
do
        LIBS="-lpthread -l$bdb_lib $old_LIBS"
        AC_TRY_LINK( [ #include <db_cxx.h> ],
                     [ Db db(NULL,0); ], [ BDB_LIBS="-l$bdb_lib"; break ] )
done
if test "x$BDB_LIBS" == "x"; then
   AC_MSG_ERROR( "Unable to find a proper version of libdb_cxx (should be one of $bdb_lib_list)" )
fi
LIBS=$old_LIBS
AC_MSG_RESULT( "LIBS used for the Berkeley DB: $BDB_LIBS" )
AC_SUBST(BDB_LIBS)
AC_LANG_RESTORE
#
# End temporary fix block
#

have_classads=no
have_boost=no
have_glite_wmsutils=no
have_globus=no
have_glite_lb=no
have_glite_security=no
have_glite_wms_common=no
have_gridsite=no
have_xml2=no
#have_db=no
have_glite_wms_purger=no

AC_OPTIMIZE
AC_GLITE

AC_GLITE_WMS_COMMON([], have_glite_wms_common=yes, have_glite_wms_common=no)
AC_MSG_RESULT(["GLITE_WMS_COMMON found $have_glite_wms_common"])

AC_CLASSADS([], have_classads=yes, have_classads=no)
AC_GSOAP([])
AC_GLOBUS([], have_globus=yes, have_globus=no)
AC_BOOST([], have_boost=yes, have_boost=no)
#AC_DB([], have_db=yes, have_db=no)
AC_GLITE_WMSUTILS([], have_glite_wmsutils=yes, have_glite_wmsutils=no)
AC_GLITE_WMS_PURGER([], have_glite_wms_purger=yes, have_glite_wms_purger=no)
AC_MSG_RESULT(["CLASSADS found $have_classads"])
AC_MSG_RESULT(["BOOST found $have_boost"])
AC_MSG_RESULT(["GLITE_WMSUTILS found $have_glite_wmsutils"])
AC_MSG_RESULT(["GLOBUS found $have_globus"])
#AC_MSG_RESULT(["DB found $have_db"])
AC_MSG_RESULT(["GLITE_WMS_PURGER found $have_glite_wms_purger"])

AC_XML2([], have_xml2=yes, have_xml2=no)
AC_MSG_RESULT(["XML2 found $have_xml2"])

AC_GLITE_LB([], have_glite_lb=yes, have_glite_lb=no)
AC_MSG_RESULT(["GLITE_LB found $have_glite_lb"])

AC_GLITE_SECURITY([], have_glite_security=yes, have_glite_security=no)
AC_MSG_RESULT(["GLITE_SECURITY found $have_glite_security"])

AC_SEC_GSOAP_PLUGIN

AC_GRIDSITE([], have_gridsite=yes, have_gridsite=no)
AC_MSG_RESULT(["GRIDSITE found $have_gridsite"])

AC_PREFIX_DEFAULT([/opt/glite])
AM_CONFIG_HEADER([src/autogen/config.h])
AC_CONFIG_FILES([config/Makefile])
AC_CONFIG_FILES([config/glite-wms-ice])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/iceUtils/Makefile])
AC_CONFIG_FILES([src/ice/Makefile])
AC_CONFIG_FILES([src/iceCommands/Makefile])
AC_CONFIG_FILES([src/iceThreads/Makefile])


AC_OUTPUT
