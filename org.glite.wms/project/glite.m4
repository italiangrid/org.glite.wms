dnl Usage:
dnl AC_GLITE
dnl - GLITE_LOCATION
dnl check for things needed by globus_ssl_utils thirdparty library
dnl (basicaly a modified code from globus_ssl_utils's configure.in file)

AC_DEFUN(AC_GLITE,
[
    AC_ARG_WITH(glite_location,
        [  --with-glite-location=PFX     prefix where GLITE is installed. (/opt/glite)],
        [],
        with_glite_location=/opt/glite)

    if test -n "with_glite_location" ; then
    	GLITE_LOCATION="$with_glite_location"
	GLITE_CFLAGS=$GLITE_LOCATION/include
    else
	GLITE_LOCATION=""
	GLITE_CFLAGS=""
    fi

    AC_MSG_RESULT([GLITE_LOCATION set to $GLITE_LOCATION])

    AC_SUBST(GLITE_LOCATION)
    AC_SUBST(GLITE_CFLAGS)
])

