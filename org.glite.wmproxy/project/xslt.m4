dnl Usage:
dnl AC_XSLT(MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for expat, and defines
dnl - XSLT_CFLAGS (compiler flags)
dnl - XSLT_LIBS (linker flags, stripping and path)
dnl - XSLT_STATIC_LIBS (linker flags, stripping and path for static library)
dnl - XSLT_INSTALL_PATH
dnl prerequisites:

AC_DEFUN([AC_XSLT],
[
    AC_ARG_WITH(xslt_prefix, 
	[  --with-xslt-prefix=PFX      prefix where 'xslt' is installed.],
	[], 
        with_xslt_prefix=${XSLT_INSTALL_PATH:-/usr})

    AC_MSG_CHECKING([for XSLT installation at ${with_xslt_prefix:-/usr}])

    ac_save_CFLAGS=$CFLAGS
    ac_save_LIBS=$LIBS
    XSLT_CFLAGS="-I$with_xslt_prefix/include"
    XSLT_LIBS="-L$with_xslt_prefix/lib64"

    XSLT_LIBS="$XSLT_LIBS -lxslt"	
    CFLAGS=$ac_save_CFLAGS
    LIBS=$ac_save_LIBS	

    AC_SUBST(XSLT_INSTALL_PATH)
    AC_SUBST(XSLT_CFLAGS)
    AC_SUBST(XSLT_LIBS)
    AC_SUBST(XSLT_STATIC_LIBS)
])

