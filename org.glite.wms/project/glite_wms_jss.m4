dnl Usage:
dnl AC_GLITE_WMS_JSS
dnl - GLITE_WMS_JSS_COMMON_LIBS
dnl - GLITE_WMS_JSS_CONTROLLER_LIBS
dnl - GLITE_WMS_JSS_CONTROLLER_ADAPTER_LIBS
dnl - GLITE_WMS_JSS_CONTROLLER_WRAPPER_LIBS
dnl - GLITE_WMS_JSS_LOGMONITOR_LIBS

AC_DEFUN(AC_GLITE_WMS_JSS,
[
    ac_glite_wms_jss_prefix=$GLITE_LOCATION

    if test -n "ac_glite_wms_jss_prefix" ; then
	dnl
	dnl 
	dnl
        ac_glite_wms_jss_lib="-L$ac_glite_wms_jss_prefix/lib"
        GLITE_WMS_JSS_COMMON_LIBS="$ac_glite_wms_jss_lib -lglite_wms_jss_common"

	GLITE_WMS_JSS_CONTROLLER_LIBS="$ac_glite_wms_jss_lib -lglite_wms_jss_controller"
	GLITE_WMS_JSS_CONTROLLER_ADAPTER_LIBS="$ac_glite_wms_jss_lib -lglite_wms_jss_controller_adapter"
	GLITE_WMS_JSS_CONTROLLER_WRAPPER_LIBS="$ac_glite_wms_jss_lib -lglite_wms_jss_controller_wrapper"
	GLITE_WMS_JSS_LOGMONITOR_LIBS="$ac_glite_wms_jss_lib -lglite_wms_jss_logmonitor"
	ifelse([$2], , :, [$2])
    else
	GLITE_WMS_JSS_COMMON_LIBS=""
	GLITE_WMS_JSS_CONTROLLER_LIBS=""
	GLITE_WMS_JSS_CONTROLLER_ADAPTER_LIBS=""
	GLITE_WMS_JSS_CONTROLLER_WRAPPER_LIBS=""
	GLITE_WMS_JSS_LOGMONITOR_LIBS=""
	ifelse([$3], , :, [$3])
    fi

    AC_SUBST(GLITE_WMS_JSS_COMMON_LIBS)
    AC_SUBST(GLITE_WMS_JSS_CONTROLLER_LIBS)
    AC_SUBST(GLITE_WMS_JSS_CONTROLLER_ADAPTER_LIBS)
    AC_SUBST(GLITE_WMS_JSS_CONTROLLER_WRAPPER_LIBS)
    AC_SUBST(GLITE_WMS_JSS_LOGMONITOR_LIBS)
])

