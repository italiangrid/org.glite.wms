#ifndef GLITE_WMS_COMMON_UTILITIES_QUOTA_H
#define GLITE_WMS_COMMON_UTILITIES_QUOTA_H

#include <string>

namespace glite {
namespace wms {
namespace common {
namespace utilities {
namespace quota {

  extern std::pair<long, long> getFreeQuota(const std::string&);
  extern std::pair<long, long> getQuota(const std::string&);

  extern uid_t user2uid(const char *name);

} // namespace quota
} // namespace utilities
} // namespace common
} // namespace wms
} // namespace glite

// Local Variables:
// mode: c++
// End:
// 

#endif /* GLITE_WMS_COMMON_UTILITIES_QUOTA_H */
