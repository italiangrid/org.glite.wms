dnl Usage:
dnl AC_GLITE_WMS_TLS
dnl - GLITE_WMS_TLS_CFLAGS
dnl - GLITE_WMS_TLS_SOCKET_LIBS
dnl - GLITE_WMS_TLS_GSI_SOCKET_LIBS
dnl - GLITE_WMS_TLS_GSISOCKET_LIBS
dnl - GLITE_WMS_TLS_SSL_HELPERS_LIBS
dnl - GLITE_WMS_TLS_SSL_THR_HELPERS_LIBS

AC_DEFUN(AC_GLITE_WMS_TLS,
[
    ac_glite_wms_tls_prefix=$GLITE_LOCATION

    if test -n "ac_glite_wms_tls_prefix" ; then
    	GLITE_WMS_TLS_CFLAGS="-I$ac_glite_wms_tls_prefix/include/glite/wms/tls"
	dnl
	dnl 
	dnl
        ac_glite_wms_tls_lib="-L$ac_glite_wms_tls_prefix/lib"
	GLITE_WMS_TLS_SOCKET_LIBS="$ac_glite_wms_tls_lib -lglite_wms_tls_socket_pp"
	GLITE_WMS_TLS_GSI_SOCKET_LIBS="$ac_glite_wms_tls_lib -lglite_wms_tls_gsisocket_pp"
	GLITE_WMS_TLS_GSISOCKET_LIBS="$GLITE_WMS_TLS_SOCKET_LIBS -lglite_wms_tls_gsisocket_pp"
	GLITE_WMS_TLS_SSL_HELPERS_LIBS="$ac_glite_wms_tls_lib -lglite_wms_tls_ssl_helpers"
	GLITE_WMS_TLS_SSL_THR_HELPERS_LIBS="$ac_glite_wms_tls_lib -lglite_wms_tls_ssl_pthr_helpers"
	ifelse([$2], , :, [$2])
    else
	GLITE_WMS_TLS_CFLAGS=""
	GLITE_WMS_TLS_SOCKET_LIBS=""
	GLITE_WMS_TLS_GSI_SOCKET_LIBS=""
	GLITE_WMS_TLS_GSISOCKET_LIBS=""
	GLITE_WMS_TLS_SSL_HELPERS_LIBS=""
	GLITE_WMS_TLS_SSL_THR_HELPERS_LIBS=""
	ifelse([$3], , :, [$3])
    fi

    AC_SUBST(GLITE_WMS_TLS_CFLAGS)
    AC_SUBST(GLITE_WMS_TLS_SOCKET_LIBS)
    AC_SUBST(GLITE_WMS_TLS_GSI_SOCKET_LIBS)
    AC_SUBST(GLITE_WMS_TLS_GSISOCKET_LIBS)
    AC_SUBST(GLITE_WMS_TLS_SSL_HELPERS_LIBS)
    AC_SUBST(GLITE_WMS_TLS_SSL_THR_HELPERS_LIBS)
])

