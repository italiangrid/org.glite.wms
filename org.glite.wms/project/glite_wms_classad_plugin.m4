dnl Usage:
dnl AC_GLITE_WMS_CLASSAD_PLUGIN
dnl - GLITE_WMS_CLASSAD_PLUGIN_LIBS

AC_DEFUN(AC_GLITE_WMS_CLASSAD_PLUGIN,
[
    ac_glite_wms_cp_prefix=$GLITE_LOCATION

    if test -n "ac_glite_wms_cp_prefix" ; then
	dnl
	dnl 
	dnl
        ac_glite_wms_cp_lib="-L$ac_glite_wms_cp_prefix/lib"
	GLITE_WMS_CLASSAD_PLUGIN_LIBS="$ac_glite_wms_cp_lib -lglite_wms_classad_plugin_loader"
	ifelse([$2], , :, [$2])
    else
	GLITE_WMS_CLASSAD_PLUGIN_LIBS=""
	ifelse([$3], , :, [$3])
    fi

    AC_SUBST(GLITE_WMS_CLASSAD_PLUGIN_LIBS)
])

