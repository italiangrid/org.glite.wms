#ifndef GLITE_WMS_JOBID_MANIPULATION_H
#define GLITE_WMS_JOBID_MANIPULATION_H

#include <string>

namespace glite {
namespace wms {
namespace jobid {

class JobId;

std::string get_reduced_part( const JobId &id, int level = 0 );
std::string to_filename( const JobId &id );
JobId from_filename( const std::string &filename );

} // namespace jobid
} // namespace wms
} // namespace glite

#endif /* GLITE_WMS_JOBID_MANIPULATION_H */

// Local Variables:
// mode: c++
// End:
