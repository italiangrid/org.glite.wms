dnl Usage:
dnl AC_GLITE_WMSUTILS_JOBID
dnl - GLITE_WMSUTILS_JOBID_CFLAGS
dnl - GLITE_WMSUTILS_JOBID_LIBS

AC_DEFUN(AC_GLITE_WMSUTILS_JOBID,
[
    ac_glite_wmsutils_jobid_prefix=$GLITE_LOCATION

    if test -n "ac_glite_wmsutils_jobid_prefix" ; then
    	GLITE_WMSUTILS_JOBID_CFLAGS="-I$ac_glite_wmsutils_jobid_prefix/include/glite/wmsutils/jobid"
	dnl
	dnl path only for 
	dnl
        ac_glite_wmsutils_jobid_lib="-L$ac_glite_wmsutils_jobid_prefix/lib"
	GLITE_WMSUTILS_JOBID_LIBS="$ac_glite_wmsutils_jobid_lib -lglite_wmsutils_jobid"
	GLITE_WMSUTILS_CJOBID_LIBS="$ac_glite_wmsutils_jobid_lib -lglite_wmsutils_cjobid"
	ifelse([$2], , :, [$2])
    else
	GLITE_WMSUTILS_JOBID_CFLAGS=""
	GLITE_WMSUTILS_JOBID_LIBS=""
	GLITE_WMSUTILS_CJOBID_LIBS=""
	ifelse([$3], , :, [$3])
    fi

    AC_SUBST(GLITE_WMSUTILS_JOBID_CFLAGS)
    AC_SUBST(GLITE_WMSUTILS_JOBID_LIBS)
    AC_SUBST(GLITE_WMSUTILS_CJOBID_LIBS)
])

