/* Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners/ for details on the copyright
holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */
#ifndef EDG_WORKLOAD_JOBCONTROL_CONTROLLER_JOBCONTROLLERCLIENTREAL_H
#define EDG_WORKLOAD_JOBCONTROL_CONTROLLER_JOBCONTROLLERCLIENTREAL_H

#include <memory>

#include "glite/wms/common/utilities/FileList.h"
#include "glite/wms/common/utilities/FileListLock.h"
#include "glite/wms/common/utilities/Extractor.h"

#include "JobControllerClientImpl.h"
#include "Request.h"

JOBCONTROL_NAMESPACE_BEGIN {

namespace controller {

class JobControllerClientReal : public JobControllerClientImpl {
public:
  JobControllerClientReal( void );
  virtual ~JobControllerClientReal( void );

  virtual void release_request( void );
  virtual void extract_next_request( void );
  virtual const Request *get_current_request( void );

private:
  typedef glite::wms::common::utilities::FileList<classad::ClassAd>   queue_type;

  bool                                                        jccr_currentGood;
  queue_type::iterator                                        jccr_current;
  Request                                                     jccr_request;
  queue_type                                                  jccr_queue;
  std::auto_ptr<glite::wms::common::utilities::FileListDescriptorMutex>   jccr_mutex;
  glite::wms::common::utilities::ForwardExtractor<queue_type>             jccr_extractor;
};

class JobControllerClientUnknown : public JobControllerClientImpl {
public:
  JobControllerClientUnknown( void );
  virtual ~JobControllerClientUnknown( void );

  virtual void release_request( void );
  virtual void extract_next_request( void );
  virtual const Request *get_current_request( void );

private:
  Request    jccu_request;
};

} // namespace controller

} JOBCONTROL_NAMESPACE_END

#endif /* EDG_WORKLOAD_JOBCONTROL_CONTROLLER_JOBCONTROLLERCLIENTREAL_H */

// Local Variables:
// mode: c++
// End:
