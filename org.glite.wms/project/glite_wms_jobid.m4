dnl Usage:
dnl AC_GLITE_WMS_JOBID
dnl - GLITE_WMS_JOBID_CFLAGS
dnl - GLITE_WMS_JOBID_LIBS

AC_DEFUN(AC_GLITE_WMS_JOBID,
[
    ac_glite_wms_jobid_prefix=$GLITE_LOCATION

    if test -n "ac_glite_wms_jobid_prefix" ; then
    	GLITE_WMS_JOBID_CFLAGS="-I$ac_glite_wms_jobid_prefix/include/glite/wms/jobid"
	dnl
	dnl path only for 
	dnl
        ac_glite_wms_jobid_lib="-L$ac_glite_wms_jobid_prefix/lib"
	GLITE_WMS_JOBID_LIBS="$ac_glite_wms_jobid_lib -lglite_wms_jobid"
	GLITE_WMS_CJOBID_LIBS="$ac_glite_wms_jobid_lib -lglite_wms_cjobid"
	ifelse([$2], , :, [$2])
    else
	GLITE_WMS_JOBID_CFLAGS=""
	GLITE_WMS_JOBID_LIBS=""
	GLITE_WMS_CJOBID_LIBS=""
	ifelse([$3], , :, [$3])
    fi

    AC_SUBST(GLITE_WMS_JOBID_CFLAGS)
    AC_SUBST(GLITE_WMS_JOBID_LIBS)
    AC_SUBST(GLITE_WMS_CJOBIS_LIBS)
])

