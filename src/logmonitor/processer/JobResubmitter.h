#ifndef EDG_WORKLOAD_JOBCONTROL_LOGMONITOR_PROCESSER_JOBRESUBMITTER_H
#define EDG_WORKLOAD_JOBCONTROL_LOGMONITOR_PROCESSER_JOBRESUBMITTER_H

#include <classad_distribution.h>

#include "glite/wms/common/utilities/FileList.h"
#include "../../common/IdContainer.h"

JOBCONTROL_NAMESPACE_BEGIN {

namespace jccommon { 
  class EventLogger;
}

namespace logmonitor { namespace processer {

class JobResubmitter {
  typedef  glite::wms::common::utilities::FileList<classad::ClassAd>   FileList;

public:
  JobResubmitter( jccommon::EventLogger *logger );
  ~JobResubmitter( void );

  void resubmit( int laststatus, const std::string &edgid, const std::string &sequence_code, jccommon::IdContainer *container = NULL );

private:
  FileList                 jr_list;
  jccommon::EventLogger   *jr_logger;
};

}}; // Namespace processer, logmonitor

} JOBCONTROL_NAMESPACE_END;

#endif /* EDG_WORKLOAD_JOBCONTROL_LOGMONITOR_PROCESSER_JOBRESUBMITTER_H */

// Local Variables:
// mode: c++
// End:
