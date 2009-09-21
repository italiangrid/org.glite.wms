#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdio>
#include <ctime>

#include <string>
#include <iostream>
#include <fstream>

#include <boost/lexical_cast.hpp>
#include "glite/wms/common/logger/logstream.h"
#include "glite/wms/common/logger/manipulators.h"
#include "common/EventLogger.h"

#include "EventJobReleased.h"
#include "MonitorData.h"

USING_COMMON_NAMESPACE;
RenameLogStreamNS(elog);

JOBCONTROL_NAMESPACE_BEGIN {

namespace logmonitor {
namespace processer {

EventJobReleased::EventJobReleased(ULogEvent *event, MonitorData *data)
  : EventInterface(event, data), event_(dynamic_cast<JobReleasedEvent*>(event))
{ }

void EventJobReleased::process_event()
{
  logger::StatePusher pusher(elog::cedglog, "EventJobReleased::process_event()");

  elog::cedglog << logger::setlevel(logger::info) << "Got a job release event for cluster: " << ei_condor << '\n';

  return;
}

}} // namespace processer, logmonitor
} JOBCONTROL_NAMESPACE_END;
