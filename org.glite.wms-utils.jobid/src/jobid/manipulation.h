#ifndef EDG_WORKLOAD_COMMON_JOBID_MANIPULATION_H
#define EDG_WORKLOAD_COMMON_JOBID_MANIPULATION_H

#include <string>

#include "../common_namespace.h"

COMMON_NAMESPACE_BEGIN {

namespace jobid {

class JobId;

std::string get_reduced_part( const JobId &id, int level = 0 );
std::string to_filename( const JobId &id );
JobId from_filename( const std::string &filename );

}; // Namespace jobid

} COMMON_NAMESPACE_END;

#endif /* EDG_WORKLOAD_COMMON_JOBID_MANIPULATION_H */

// Local Variables:
// mode: c++
// End:
