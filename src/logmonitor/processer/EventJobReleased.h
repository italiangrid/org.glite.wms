#ifndef EDG_WORKLOAD_JOBCONTROL_LOGMONITOR_PROCESSER_EVENTJOBRELEASED_H
#define EDG_WORKLOAD_JOBCONTROL_LOGMONITOR_PROCESSER_EVENTJOBRELEASED_H

#include "EventInterface.h"
#include <user_log.c++.h>

JOBCONTROL_NAMESPACE_BEGIN {

namespace logmonitor {
namespace processer {

struct MonitorData;
class EventFactory;

class EventJobReleased : public EventInterface {
  friend class EventFactory;

public:
  virtual ~EventJobReleased() { }
  virtual void process_event();

private:
  EventJobReleased(ULogEvent *event, MonitorData *data);
  JobReleasedEvent *event_;
};

}} // Namespace processer, logmonitor

} JOBCONTROL_NAMESPACE_END

#endif /* EDG_WORKLOAD_JOBCONTROL_LOGMONITOR_PROCESSER_EVENTJOBHELD_H */

// Local Variables:
// mode: c++
// End:
