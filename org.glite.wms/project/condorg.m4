dnl Usage:
dnl AC_CONDORG(MINIMUM-VERSION [, ACTION-IF-FOUND [, ACTION_IF_NOT_FOUND]])
dnl Tests for CondorG libraries and includes, defines:
dnl - CONDORG_CFLAGS (compiler flags)
dnl - CONDORG_LIBS (linker flags, stripping and path)
dnl - CONDORG_INSTALL_PATH
dnl prerequisites:

AC_DEFUN(AC_CONDORG,
[
    AC_ARG_WITH(condor_prefix,
	[  --with-condor-prefix=PFX    prefix where CondorG is installed.],
	[], 
        with_condor_prefix=${CONDORG_INSTALL_PATH:-/opt/condor})

    AC_MSG_CHECKING([for CONDOR installation])

    CONDORG_CFLAGS=""
    CONDORG_LIBS="-lcondorapi -ldl"

    if test -n "$with_condor_prefix" ; then
      AC_MSG_RESULT([prefix: $with_condor_prefix])

      ac_condor_library_prefix="$with_condor_prefix/lib"
      ac_condor_include_prefix="$with_condor_prefix/include"
      CONDORG_CFLAGS="-I$ac_condor_include_prefix"
      CONDORG_LIBS="-L$ac_condor_library_prefix $CONDORG_LIBS"
    fi

    CONDOR_BIN=$with_condor_prefix/bin

    AC_PATH_PROG(RUN_CONDOR_VER,condor_version,no,$CONDOR_BIN)

    if test "$RUN_CONDOR_VER" != "no" -a -x $RUN_CONDOR_VER ; then
	$RUN_CONDOR_VER | egrep '^\$CondorVersion' &> condor.ver

	changequote(<<, >>)
	condor_major_ver=`cat condor.ver | sed 's/.*Version: \([0-9]\+\)\.[0-9]\+\.[0-9]\+.*/\1/p; d'`
	condor_minor_ver=`cat condor.ver | sed 's/.*Version: [0-9]\+\.\([0-9]\+\)\.[0-9]\+.*/\1/p; d'`
	condor_micro_ver=`cat condor.ver | sed 's/.*Version: [0-9]\+\.[0-9]\+\.\([0-9]\+\).*/\1/p; d'`

	ac_req_major=`echo "$1" | sed 's/\([0-9]\+\)\.[0-9]\+\.[0-9]\+.*/\1/p; d'`
	ac_req_minor=`echo "$1" | sed 's/[0-9]\+\.\([0-9]\+\)\.[0-9]\+.*/\1/p; d'`
	ac_req_micro=`echo "$1" | sed 's/[0-9]\+\.[0-9]\+\.\([0-9]\+\).*/\1/p; d'`
	changequote([, ])

	rm condor.ver

        CONDORG_VERSION=$condor_major_ver$condor_minor_ver$condor_micro_ver

        AC_DEFINE_UNQUOTED(CONDORG_VERSION, $CONDORG_VERSION)
        
	if test $condor_major_ver -gt $ac_req_major -o \
		$condor_major_ver -eq $ac_req_major -a $condor_minor_ver -gt $ac_req_minor -o \
		$condor_major_ver -eq $ac_req_major -a $condor_minor_ver -eq $ac_req_minor -a $condor_micro_ver -ge $ac_req_micro ; then

	    AC_LANG_SAVE
	    AC_LANG_CPLUSPLUS
	    ac_save_cppflags=$CPPFLAGS
	    ac_save_libs=$LIBS
	    CPPFLAGS="$CONDORG_CFLAGS $CPPFLAGS"
	    LIBS="$CONDORG_LIBS $LIBS"
	    AC_MSG_CHECKING([if a small Condor log parser compiles])
	    AC_TRY_LINK([ #include <ctime>
			  #include <cstdio>
			  #include <user_log.c++.h> ],
			[ ReadUserLog     rul;],
			[ ac_have_condorlibs=yes ], [ ac_have_condorlibs=no ] )
	    AC_MSG_RESULT([$ac_have_condorlibs])
	    CPPFLAGS=$ac_save_cppflags
	    LIBS=$ac_save_libs
	    AC_LANG_RESTORE

	    if test "$ac_have_condorlibs" = "yes" ; then
		ac_cv_condor_valid=yes
	    else
		ac_cv_condor_valid=no
		AC_MSG_WARN([
		    ***  Cannot compile a small CondorG log parser: check
		    ***  whether the CondorG parsing libraries are installed])
	    fi
	else
	    ac_cv_condor_valid=no
	    AC_MSG_WARN([***  Mismatching CondorG version, please upgrade to $1])
	fi
    else
	ac_cv_condor_valid=no
	AC_MSG_WARN([
	    ***  Cannot find an usable condor_version program: please check
	    ***  your CondorG installation])
    fi

    AC_MSG_RESULT([Condor version: $CONDORG_VERSION])

    if test x$ac_cv_condor_valid = xyes ; then
	CONDORG_INSTALL_PATH=$with_condor_prefix
        ifelse([$2], , :, [$2])
    else
        CONDORG_CFLAGS=""
        CONDORG_LIBS=""
        CONDORG_VERSION=""
        ifelse([$3], , :, [$3])
    fi

    AC_SUBST(CONDORG_INSTALL_PATH)
    AC_SUBST(CONDORG_CFLAGS)
    AC_SUBST(CONDORG_LIBS)
])

