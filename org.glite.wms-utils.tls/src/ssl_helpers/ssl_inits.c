#ident "$Header$"

#include <openssl/ssl.h>

#include "../ssl_helpers/ssl_inits.h"

#include "sslutils.h"

int edg_wlc_SSLInitialization(void)
{
	/* Initialize OpenSSL library */

	SSL_load_error_strings();
	SSL_library_init();

	/* Initialize Globus ssl_utils library,
	 * includes random number generator initialization */

	ERR_load_prxyerr_strings(1);

	return 0;
}
