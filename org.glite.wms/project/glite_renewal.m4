dnl Usage:
dnl AC_GLITE_SECURITY
dnl - GLITE_SECURITY_THR_RENEWAL_LIBS
dnl - GLITE_SECURITY_NOTHR_RENEWAL_LIBS

AC_DEFUN(AC_GLITE_SECURITY,
[
    ac_glite_security_prefix=$GLITE_LOCATION

    if test -n "ac_glite_security_prefix" ; then
	dnl
	dnl 
	dnl
        ac_glite_security_lib="-L$ac_glite_security_prefix/lib"
	GLITE_SECURITY_THR_RENEWAL_LIBS="$ac_glite_security_lib -lglite_securityproxyrenewal_$GLOBUS_THR_FLAVOR"

	GLITE_SECURITY_NOTHR_RENEWAL_LIBS="$ac_glite_security_lib -lglite_security_proxyrenewal_$GLOBUS_NOTHR_FLAVOR"

	ifelse([$2], , :, [$2])
    else
	GLITE_SECURITY_THR_RENEWAL_LIBS=""
	GLITE_SECURITY_NOTHR_RENEWAL_LIBS=""
	ifelse([$3], , :, [$3])
    fi

    AC_SUBST(GLITE_SECURITY_THR_RENEWAL_LIBS)
    AC_SUBST(GLITE_SECURITY_NOTHR_RENEWAL_LIBS)
])

