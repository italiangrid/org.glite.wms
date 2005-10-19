#include <string>
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/gpbox/Clientcc.h"
#include "glite/wms/matchmaking/matchmaker.h"

namespace jobid         = glite::wmsutils::jobid;
namespace matchmaking   = glite::wms::matchmaking;

namespace glite {
namespace wms {
namespace helper {
namespace gpbox_utils {

extern std::string get_user_x509_proxy(jobid::JobId const& jobid);
extern std::string get_proxy_distinguished_name(std::string const& proxy_file);
extern bool filter_gpbox_authorizations(matchmaking::match_table_t& suitable_CEs,
                                 Connection& PEP_connection,
                                 std::string const& user_cert_file_name);
}}}}
