dnl Usage:
dnl AC_GLITE_WMSUTILS_EXCEPTION
dnl - GLITE_WMSUTILS_EXCEPTION_CFLAGS
dnl - GLITE_WMSUTILS_EXCEPTION_LIBS

AC_DEFUN(AC_GLITE_WMSUTILS_EXCEPTION,
[
    ac_glite_wmsutils_exception_prefix=$GLITE_LOCATION

    if test -n "ac_glite_wmsutils_exception_prefix" ; then
    	GLITE_WMSUTILS_EXCEPTION_CFLAGS="-I$ac_glite_wmsutils_exception_prefix/include/glite/wmsutils/exception"
	dnl
	dnl path only for 
	dnl
        ac_glite_wmsutils_exception_lib="-L$ac_glite_wmsutils_exception_prefix/lib"
	GLITE_WMSUTILS_EXCEPTION_LIBS="$ac_glite_wmsutils_exception_lib -lglite_wmsutils_exception"
	ifelse([$2], , :, [$2])
    else
	GLITE_WMSUTILS_EXCEPTION_CFLAGS=""
	GLITE_WMSUTILS_EXCEPTION_LIBS=""
	ifelse([$3], , :, [$3])
    fi

    AC_SUBST(GLITE_WMSUTILS_EXCEPTION_CFLAGS)
    AC_SUBST(GLITE_WMSUTILS_EXCEPTION_LIBS)
])

