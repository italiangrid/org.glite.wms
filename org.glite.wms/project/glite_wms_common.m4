dnl Usage:
dnl AC_GLITE_WMS_COMMON
dnl - GLITE_WMS_COMMON_CFLAGS
dnl - GLITE_WMS_COMMON_LIBS
dnl check for things needed by globus_ssl_utils thirdparty library
dnl (basicaly a modified code from globus_ssl_utils's configure.in file)

AC_DEFUN(AC_GLITE_WMS_COMMON,
[
    ac_glite_wms_common_prefix=$GLITE_LOCATION

    if test -n "ac_glite_wms_common_prefix" ; then
    	GLITE_WMS_COMMON_CFLAGS="-I$ac_glite_wms_common_prefix/include/glite/wms/common"
	dnl
	dnl path only for workload/logging/common
	dnl
        ac_glite_wms_common_lib="-L$ac_glite_wms_common_prefix/lib"
	GLITE_WMS_COMMON_CONF_LIBS="$ac_glite_wms_common_lib -lglite_wms_conf"
	GLITE_WMS_COMMON_CONF_WRAPPER_LIBS="$ac_glite_wms_common_lib -lglite_wms_conf_wrapper"
	GLITE_WMS_COMMON_CONFIG_LIBS="$GLITE_WMS_COMMON_CONF_LIBS -lglite_wms_conf_wrapper"
	GLITE_WMS_COMMON_LDIF2CLASSADS_LIBS="$ac_glite_wms_common_lib -lglite_wms_LDIF2ClassAd"
	GLITE_WMS_COMMON_LOGGER_LIBS="$ac_glite_wms_common_lib -lglite_wms_logger"
	GLITE_WMS_COMMON_PROCESS_LIBS="$ac_glite_wms_common_lib -lglite_wms_process"
	GLITE_WMS_COMMON_UT_EXCEPTIONS_LIBS="$ac_glite_wms_common_lib -lglite_wms_exceptions"
	GLITE_WMS_COMMON_UT_UTIL_LIBS="$ac_glite_wms_common_lib -lglite_wms_util"
	GLITE_WMS_COMMON_UT_FTP_LIBS="$ac_glite_wms_common_lib -lglite_wms_globus_ftp_util"
	GLITE_WMS_COMMON_UT_II_LIBS="$ac_glite_wms_common_lib -lglite_wms_iiattrutil"
	ifelse([$2], , :, [$2])
    else
	GLITE_WMS_COMMON_CFLAGS=""
	GLITE_WMS_COMMON_CONF_LIBS=""
	GLITE_WMS_COMMON_CONF_WRAPPER_LIBS=""
	GLITE_WMS_COMMON_CONFIG_LIBS=""
	GLITE_WMS_COMMON_LDIF2CLASSADS_LIBS=""
	GLITE_WMS_COMMON_LOGGER_LIBS=""
	GLITE_WMS_COMMON_PROCESS_LIBS=""
	GLITE_WMS_COMMON_UT_EXCEPTIONS_LIBS=""
	GLITE_WMS_COMMON_UT_UTIL_LIBS=""
	GLITE_WMS_COMMON_UT_FTP_LIBS=""
	GLITE_WMS_COMMON_UT_II_LIBS=""
	ifelse([$3], , :, [$3])
    fi

    AC_SUBST(GLITE_WMS_COMMON_CFLAGS)
    AC_SUBST(GLITE_WMS_COMMON_CONF_LIBS)
    AC_SUBST(GLITE_WMS_COMMON_CONF_WRAPPER_LIBS)
    AC_SUBST(GLITE_WMS_COMMON_CONFIG_LIBS)
    AC_SUBST(GLITE_WMS_COMMON_LDIF2CLASSADS_LIBS)
    AC_SUBST(GLITE_WMS_COMMON_LOGGER)
    AC_SUBST(GLITE_WMS_COMMON_PROCESS)
    AC_SUBST(GLITE_WMS_COMMON_UT_EXCEPTIONS_LIBS)
    AC_SUBST(GLITE_WMS_COMMON_UT_UTIL_LIBS)
    AC_SUBST(GLITE_WMS_COMMON_UT_FTP_LIBS)
    AC_SUBST(GLITE_WMS_COMMON_UT_II_LIBS)	
])

