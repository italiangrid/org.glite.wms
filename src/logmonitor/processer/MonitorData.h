#ifndef EDG_WORKLOAD_JOBCONTROL_LOGMONITOR_PROCESSER_MONITORDATA_H
#define EDG_WORKLOAD_JOBCONTROL_LOGMONITOR_PROCESSER_MONITORDATA_H

#include <memory>
#include <string>

JOBCONTROL_NAMESPACE_BEGIN {

namespace jccommon {
  class EventLogger;
  class IdContainer;
}

namespace logmonitor { 

namespace processer {
  class JobResubmitter;
}
class AbortedContainer;
class Timer;
class SizeFile;

struct MonitorData {
  MonitorData( AbortedContainer *aborted, jccommon::IdContainer *container, processer::JobResubmitter *resubmitter, jccommon::EventLogger *logger = NULL );

  jccommon::EventLogger       *md_logger;
  jccommon::IdContainer       *md_container;
  AbortedContainer            *md_aborted;
  processer::JobResubmitter   *md_resubmitter;
};

namespace processer {

struct MonitorData {
  MonitorData( const char *filename, logmonitor::MonitorData &data );
  MonitorData( const std::string &filename, logmonitor::MonitorData &data );

  bool                     md_isDagLog;
  jccommon::EventLogger   *md_logger;
  jccommon::IdContainer   *md_container;
  AbortedContainer        *md_aborted;
  JobResubmitter          *md_resubmitter;
  std::string              md_logfile_name, md_dagId;
  std::auto_ptr<Timer>     md_timer;
  std::auto_ptr<SizeFile>  md_sizefile;
};

}} // Namespace processer, logmonitor

} JOBCONTROL_NAMESPACE_END;

#endif /* EDG_WORKLOAD_JOBCONTROL_LOGMONITOR_PROCESSER_MONITORDATA_H */

// Local Variables:
// mode: c++
// End:
