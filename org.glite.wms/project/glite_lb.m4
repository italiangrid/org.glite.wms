dnl Usage:
dnl AC_GLITE_LB
dnl - GLITE_LB_CFLAGS
dnl - GLITE_LB_THR_CLIENT_LIBS
dnl - GLITE_LB_THR_COMMON_LIBS
dnl - GLITE_LB_CLIENT_LIBS
dnl - GLITE_LB_COMMON_LIBS

AC_DEFUN(AC_GLITE_LB,
[
    ac_glite_lb_prefix=$GLITE_LOCATION

    if test -n "ac_glite_lb_prefix" ; then
    	GLITE_LB_CFLAGS="-I$ac_glite_lb_prefix/include/glite/lb"
	dnl
	dnl 
	dnl
        ac_glite_lb_lib="-L$ac_glite_lb_prefix/lib"
	GLITE_LB_THR_CLIENT_LIBS="$ac_glite_lb_lib -lglite_lb_client_$GLOBUS_THR_FLAVOR"
	GLITE_LB_THR_COMMON_LIBS="$ac_glite_lb_lib -lglite_lb_common_$GLOBUS_THR_FLAVOR"
	GLITE_LB_CLIENT_LIBS="$ac_glite_lb_lib -lglite_lb_client_$GLOBUS_FLAVOR"
        GLITE_LB_COMMON_LIBS="$ac_glite_lb_lib -lglite_lb_common_$GLOBUS_FLAVOR"

	ifelse([$2], , :, [$2])
    else
	GLITE_LB_CFLAGS=""
	GLITE_LB_THR_CLIENT_LIBS=""
	GLITE_LB_THR_COMMON_LIBS=""
	GLITE_LB_CLIENT_LIBS=""
	GLITE_LB_COMMON_LIBS=""
	ifelse([$3], , :, [$3])
    fi

    AC_SUBST(GLITE_LB_CFLAGS)
    AC_SUBST(GLITE_LB_THR_CLIENT_LIBS)
    AC_SUBST(GLITE_LB_THR_COMMON_LIBS)
    AC_SUBST(GLITE_LB_CLIENT_LIBS)
    AC_SUBST(GLITE_LB_COMMON__LIBS)
])

