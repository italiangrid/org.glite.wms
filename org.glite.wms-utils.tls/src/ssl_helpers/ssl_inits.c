#ident "$Header$"

#include <openssl/ssl.h>

#include "ssl_inits.h"
//#include "glite/wmsutils/thirdparty/globus_ssl_utils/sslutils.h"

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
