#ifndef EDG_WORKLOAD_JOBCONTROL_CONTROLLER_JOBCONTROLLERCLIENT_H
#define EDG_WORKLOAD_JOBCONTROL_CONTROLLER_JOBCONTROLLERCLIENT_H

JOBCONTROL_NAMESPACE_BEGIN {

namespace controller {

class Request;
class JobControllerClientImpl;

class JobControllerClient {
public:
  JobControllerClient( void );
  ~JobControllerClient( void );

  void release_request( void );
  void extract_next_request( void );
  const Request *get_current_request( void );

private:
  JobControllerClient( const JobControllerClient & ); // Not implemented
  JobControllerClient &operator=( const JobControllerClient & ); // Not implemented

  JobControllerClientImpl   *jcc_impl;
};

} // Namespace controller

} JOBCONTROL_NAMESPACE_END

#endif /* EDG_WORKLOAD_JOBCONTROL_CONTROLLER_JOBCONTROLLERCLIENT_H */

// Local Variables:
// mode: c++
// End:
