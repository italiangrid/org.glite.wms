dnl Usage:
dnl AC_GLITE_WMS_BROKER
dnl - GLITE_WMS_BROKER_LIBS

AC_DEFUN(AC_GLITE_WMS_BROKER,
[
    ac_glite_wms_broker_prefix=$GLITE_LOCATION

    if test -n "ac_glite_wms_boker_prefix" ; then
	dnl
	dnl 
	dnl
        ac_glite_wms_broker_lib="-L$ac_glite_wms_broker_prefix/lib"
	GLITE_WMS_BROKER_LIBS="$ac_glite_wms_broker_lib -lglite_wms_broker"
	ifelse([$2], , :, [$2])
    else
	GLITE_WMS_BROKER_LIBS=""
	ifelse([$3], , :, [$3])
    fi

    AC_SUBST(GLITE_WMS_BROKER_LIBS)
])

