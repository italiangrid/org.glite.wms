dnl Usage:
dnl AC_GLITE_WMSUI_PARTITIONER
dnl - GLITE_WMSUI_PARTITIONER_CFLAGS
dnl - GLITE_WMSUI_PARTITIONER_LIBS

AC_DEFUN(AC_GLITE_WMSUI_PARTITIONER,
[
    ac_glite_wmsui_part_prefix=$GLITE_LOCATION

    if test -n "ac_glite_wmsui_part_prefix" ; then
        GLITE_WMSUI_PARTITIONER_CFLAGS="-I$ac_glite_ui_part_prefix/include/glite/wmsui/partitioner"
        dnl
        dnl
        dnl
        ac_glite_wmsui_part_lib="-L$ac_glite_wmsui_part_prefix/lib"
        GLITE_WMSUI_PARTITIONER_LIBS="$ac_glite_wmsui_part_lib -lglite_wmsui_partitioner"
        ifelse([$2], , :, [$2])
    else
        GLITE_WMSUI_PARTITIONER_CFLAGS=""
        GLITE_WMSUI_PARTITIONER_LIBS=""
        ifelse([$3], , :, [$3])
    fi

    AC_SUBST(GLITE_WMSUI_PARTITIONER_CFLAGS)
    AC_SUBST(GLITE_WMSUI_PARTITIONER_LIBS)
])
