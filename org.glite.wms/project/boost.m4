dnl Usage:
dnl AC_BOOST(MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for boost, and defines
dnl - BOOST_CFLAGS (compiler flags)
dnl - BOOST_LIBS (linker flags, stripping and path)
dnl - BOOST_FS_LIBS (linker flags, stripping and path)
dnl - BOOST_THREAD_LIBS (linker flags, stripping and path)
dnl - BOOST_REGEX_LIBS (linker flags, stripping and path)
dnl - BOOST_PYTHON_LIBS (linker flags, stripping and path)
dnl - BOOST_INSTALL_PATH
dnl prerequisites:

AC_DEFUN(AC_BOOST,
[
    AC_ARG_WITH(boost_prefix, 
	[  --with-boost-prefix=PFX      prefix where Boost is installed.],
	[], 
        with_boost_prefix="/opt/boost" )

    AC_ARG_ENABLE(boost_debug,
	[  --enable-boost_debug=<option> Enable debug Boost libraries.],
	ac_boost_flavor="$enableval", 
        ac_boost_flavor="yes")

    if test x$ac_boost_flavor = xyes ; then
        ac_boost_flavor="debug"
    elif test x$ac_boost_flavor = xrelease ; then
        ac_boost_flavor="release"
    else
	AC_MSG_WARN([***  boost_flavor can be either debug or release]) 
    fi

    AC_MSG_RESULT([Using boost flavor: $ac_boost_flavor])

    AC_MSG_CHECKING([for Boost installation])

    BOOST_CFLAGS=""

    BOOST_FS_LIBS="-lboost_fs"
    BOOST_THREAD_LIBS="-lboost_thread -lpthread"
    BOOST_REGEX_LIBS="-lboost_regex"
    BOOST_PYTHON_LIBS="-lboost_python"
    BOOST_LIBS="$BOOST_FS_LIBS $BOOST_THREAD_LIBS $BOOST_REGEX_LIBS"

    if test -n "$with_boost_prefix" -a "$with_boost_prefix" != "/usr" ; then
        if test -d "$with_boost_prefix" ; then
            ac_boost_prefix="$with_boost_prefix"
            AC_MSG_RESULT([prefix: $ac_boost_prefix])
        else
            ac_boost_prefix="/usr"
            AC_MSG_RESULT([prefix: $ac_boost_prefix])
            AC_MSG_WARN([***  Cannot find an installed Boost library, trying with defaults])
        fi
    elif test -z "$with_boost_prefix" ; then
        ac_boost_prefix="/usr"
        AC_MSG_RESULT([prefix: $ac_boost_prefix])
    fi

    ac_boost_libraries="$ac_boost_prefix/lib"
    ac_boost_includes="$ac_boost_prefix/include"

    if test "$ac_boost_libraries" != "/usr/lib" ; then
        ac_boost_libraries="$ac_boost_prefix/lib/$ac_boost_flavor"

        BOOST_LIBS="-L$ac_boost_libraries $BOOST_LIBS"
        BOOST_FS_LIBS="-L$ac_boost_libraries $BOOST_FS_LIBS"
        BOOST_THREAD_LIBS="-L$ac_boost_libraries $BOOST_THREAD_LIBS"
        BOOST_REGEX_LIBS="-L$ac_boost_libraries $BOOST_REGEX_LIBS"
        BOOST_PYTHON_LIBS="-L$ac_boost_libraries $BOOST_PYTHON_LIBS"
    fi

    if test "$ac_boost_includes" != "/usr/include" ; then
        BOOST_CFLAGS="-I$ac_boost_includes"
    fi

    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS
    ac_save_cppflags=$CPPFLAGS
    ac_save_libs=$LIBS
    CPPFLAGS="$BOOST_CFLAGS $CPPFLAGS"
    LIBS="$BOOST_LIBS $LIBS"
    AC_MSG_CHECKING([if a small boost program compiles])
    AC_TRY_LINK([ #include <boost/thread/mutex.hpp> ],
    		[ boost::mutex   mut; ],
		[ ac_have_boost=yes ], [ ac_have_boost=no ])
    AC_MSG_RESULT([$ac_have_boost])
    CPPFLAGS=$ac_save_cppflags
    LIBS=$ac_save_libs
    AC_LANG_RESTORE

    if test x$ac_have_boost = xyes ; then

        dnl
        dnl Check if the compiler define the stringstream object
        dnl in order to define the BOOST_NO_STRINGSTREAM macro
        dnl 

	AC_LANG_SAVE
	AC_LANG_CPLUSPLUS
	ac_save_cppflags=$CPPFLAGS
	ac_save_libs=$LIBS
	CPPFLAGS="$BOOST_CFLAGS $CPPFLAGS"
	LIBS="$BOOST_LIBS $LIBS"
	AC_MSG_CHECKING([for stringstream into c++ STL])
	AC_TRY_LINK([ #include <sstream> ],
		    [ std::stringstream sstr; ],
		    [ ac_have_stringstream=yes ], 
		    [ ac_have_stringstream=no 
		      BOOST_CFLAGS="$BOOST_CFLAGS -DBOOST_NO_STRINGSTREAM" ])
        if test x$ac_have_stringstream = xyes ; then
            AC_DEFINE(HAVE_STRINGSTREAM)
        else 
            AC_DEFINE(BOOST_NO_STRINGSTREAM)
        fi 
	AC_MSG_RESULT([$ac_have_stringstream])
	CPPFLAGS=$ac_save_cppflags
	LIBS=$ac_save_libs
	AC_LANG_RESTORE

        BOOST_INSTALL_PATH=$ac_boost_prefix

	ifelse([$2], , :, [$2])
    else    
        AC_MSG_WARN([
            ***   Cannot compile a small boost program: check wheter the boost
            ***   libraries are fully installed and try again.])
    	BOOST_CFLAGS=""
    	BOOST_LIBS=""
	BOOST_FS_LIBS=""
        BOOST_THREAD_LIBS=""
        BOOST_REGEX_LIBS=""
        BOOST_PYTHON_LIBS=""
	ifelse([$3], , :, [$3])
    fi

    AC_SUBST(BOOST_INSTALL_PATH)
    AC_SUBST(BOOST_CFLAGS)
    AC_SUBST(BOOST_LIBS)
    AC_SUBST(BOOST_FS_LIBS)
    AC_SUBST(BOOST_THREAD_LIBS)
    AC_SUBST(BOOST_REGEX_LIBS)
    AC_SUBST(BOOST_PYTHON_LIBS)
])

