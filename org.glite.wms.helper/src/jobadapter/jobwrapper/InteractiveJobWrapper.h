/***************************************************************************
 *  filename  : InteractiveJobWrapper.h
 *  authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#ifndef GLITE_WMS_HELPER_JOBADAPTER_JOBWRAPPER_INTERACTIVE_JOB_WRAPPER_H
#define GLITE_WMS_HELPER_JOBADAPTER_JOBWRAPPER_INTERACTIVE_JOB_WRAPPER_H

#ifndef GLITE_WMS_X_STRING
#define GLITE_WMS_X_STRING
#include <string>
#endif

#ifndef GLITE_WMS_X_VECTOR
#define GLITE_WMS_X_VECTOR
#include <vector>
#endif

#ifndef GLTIE_WMS_X_IOSTREAM
#define GLITE_WMS_X_IOSTREAM
#include <iostream>
#endif

#ifndef GLITE_WMS_X_UTILITY
#define GLITE_WMS_X_UTILITY
#include <utility>
#endif

#ifndef GLITE_WMS_HELPER_JOBADAPTER_URL_URL_H
#include "jobadapter/url/URL.h"
#endif

#ifndef GLITE_WMS_HELPER_JOBADAPTER_JOBWRAPPER_JOB_WRAPPER_H
#include "JobWrapper.h"
#endif

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {
namespace jobwrapper {

class InteractiveJobWrapper : public JobWrapper
{

public:

  /**
   *  Default constructor.
   *  Initializes the object passing the parameters to the job wrapper.
   *  \param job
   *  \ingroup jobadapter
   */
  InteractiveJobWrapper(const std::string& job);

  /**
   *  Default destructor.
   *  \ingroup jobadapter
   */
  virtual ~InteractiveJobWrapper(void);

protected:
  virtual std::ostream& make_bypass_transfer(std::ostream& os,
		  			     const std::string& prefix) const;

  virtual std::ostream& execute_job(std::ostream&      os,
                                    const std::string& arguments,
                                    const std::string& job,
                                    const std::string& stdi,
                                    const std::string& stdo,
                                    const std::string& stde,
                                    int                node) const;
  
};

} // namespace jobwrapper
} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_HELPER_JOBADAPTER_JOBWRAPPER_INTERACTIVE_JOB_WRAPPER_H
