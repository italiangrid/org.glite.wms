#ifndef EDG_WORKLOAD_JOBCONTROL_COMMON_CONSTANTS_H
#define EDG_WORKLOAD_JOBCONTROL_COMMON_CONSTANTS_H

JOBCONTROL_NAMESPACE_BEGIN {

namespace jccommon {

enum generic_event_t {
  null_event,
  cancelled_event,
  retry_remove,
  cannot_cancel_event,
  user_cancelled_event
};

enum job_statuses_t {
  undefined_status = -2,
  no_resubmission
};

extern const char *generic_events_string[];

} // namespace jccommon

} JOBCONTROL_NAMESPACE_END

#endif /* EDG_WORKLOAD_JOBCONTROL_COMMON_CONSTANTS_H */

// Local Variables:
// mode: c++
// End:
