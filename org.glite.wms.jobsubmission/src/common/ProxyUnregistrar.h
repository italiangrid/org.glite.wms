#ifndef EDG_WORKLOAD_JOBCONTROL_COMMON_PROXYUNREGISTRAR_H
#define EDG_WORKLOAD_JOBCONTROL_COMMON_PROXYUNREGISTRAR_H

COMMON_SUBNAMESPACE_CLASS_J(jobid, JobId );

JOBCONTROL_NAMESPACE_BEGIN {

namespace jccommon {

class ProxyUnregistrar {
public:
  ProxyUnregistrar( const std::string &id );
  ~ProxyUnregistrar( void );

  void unregister( void );

private:
  glite::wmsutils::jobid::JobId   pu_id;
};

};

} JOBCONTROL_NAMESPACE_END;

#endif /* EDG_WORKLOAD_JOBCONTROL_COMMON_PROXYUNREGISTRAR_H */

// Local Variables:
// mode: c++
// End:
