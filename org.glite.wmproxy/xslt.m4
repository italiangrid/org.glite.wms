dnl Usage:
dnl GLITE_CHECK_XSLT

dnl Exported variables (i.e. available to the various Makefile.am):
dnl XSLT_CPPFLAGS
dnl XSLT_LDFLAGS
dnl XSLT_LIBS

AC_DEFUN([GLITE_CHECK_XSLT],
[AC_ARG_WITH(
    [xslt_prefix],
    [AS_HELP_STRING(
        [--with-xslt-prefix=PFX],
        [prefix where xslt is installed  @<:@default=/usr@:>@]
    )],
    [],
    [with_xslt_prefix=/usr]
)

AC_MSG_CHECKING([for xslt])

if test -f ${with_xslt_prefix}/include/libxslt/xslt.h; then
    XSLT_CPPFLAGS="-I${with_xslt_prefix}/include"
    
    if test "x${host_cpu}" = xx86_64 -o "x${host_cpu}" = xia64 ; then
        ac_xslt_lib_dir="lib64"
    else
        ac_xslt_lib_dir="lib"
    fi

    XSLT_LDFLAGS="-L${with_xslt_prefix}/${ac_xslt_lib_dir}"
    XSLT_LIBS="-lxslt"

    AC_MSG_RESULT([yes])
else
    AC_MSG_ERROR([no])
fi

AC_SUBST(XSLT_CPPFLAGS)
AC_SUBST(XSLT_LDFLAGS)
AC_SUBST(XSLT_LIBS)
])

