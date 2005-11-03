#include <string>

#include <openssl/x509.h>

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/gpbox/Clientcc.h"
#include "glite/wms/matchmaking/matchmaker.h"

namespace jobid         = glite::wmsutils::jobid;
namespace matchmaking   = glite::wms::matchmaking;

namespace glite {
namespace wms {
namespace helper {
namespace gpbox_utils {

extern std::string
get_user_x509_proxy(jobid::JobId const&);

extern std::string 
get_proxy_distinguished_name(std::string const&);

extern bool 
filter_gpbox_authorizations(matchmaking::match_table_t&,
                                 Connection&,
                                 std::string const&);

extern X509 *
get_real_cert(X509 *, STACK_OF(X509) *);

}}}}
