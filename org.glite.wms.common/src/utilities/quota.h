#ifndef EDG_WORKLOAD_COMMON_UTILITIES_QUOTA_H
#define EDG_WORKLOAD_COMMON_UTILITIES_QUOTA_H

#include <string>

namespace edg {
namespace workload {
namespace common {
namespace utilities {
namespace quota {

  extern std::pair<long, long> getFreeQuota(const std::string&);
  extern std::pair<long, long> getQuota(const std::string&);

  extern uid_t user2uid(const char *name);

} // namespace quota
} // namespace utilities
} // namespace common
} // namespace workload
} // edg

// Local Variables:
// mode: c++
// End:
// 

#endif /* EDG_WORKLOAD_COMMON_UTILITIES_QUOTA_H */
