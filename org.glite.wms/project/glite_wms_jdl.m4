dnl Usage:
dnl AC_GLITE_WMS_JDL
dnl - GLITE_WMS_JDL_CFLAGS
dnl - GLITE_WMS_JDL_LIBS

AC_DEFUN(AC_GLITE_WMS_JDL,
[
    ac_glite_wms_jdl_prefix=$GLITE_LOCATION

    if test -n "ac_glite_wms_jdl_prefix" ; then
    	GLITE_WMS_JDL_CFLAGS="-I$ac_glite_wms_jdl_prefix/include/glite/wms/jdl"
	dnl
	dnl 
	dnl
        ac_glite_wms_jdl_lib="-L$ac_glite_wms_jdl_prefix/lib"
	GLITE_WMS_JDL_LIBS="$ac_glite_wms_jdl_lib -lglite_wms_jdl"
	ifelse([$2], , :, [$2])
    else
	GLITE_WMS_JDL_CFLAGS=""
	GLITE_WMS_JDL_LIBS=""
	ifelse([$3], , :, [$3])
    fi

    AC_SUBST(GLITE_WMS_JDL_CFLAGS)
    AC_SUBST(GLITE_WMS_JDL_LIBS)
])

