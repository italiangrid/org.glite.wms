/***************************************************************************
 *  filename  : MpiPbsJobWrapper.h
 *  authors   : Alessio Gianelle <alessio.gianelle@pd.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
 *              Rosario Peluso <rosario.peluso@pd.infn.it>
 *              Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *              Marco Cecchi <marco.cecchi@cnaf.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#ifndef GLTIE_WMS_HELPER_JOBADAPTER_JOBWRAPPER_MPI_PBS_JOB_WRAPPER_H
#define GLTIE_WMS_HELPER_JOBADAPTER_JOBWRAPPER_MPI_PBS_JOB_WRAPPER_H

#ifndef GLTIE_WMS_X_STRING
#define GLITE_WMS_X_STRING
#include <string>
#endif

#ifndef GLITE_WMS_X_VECTOR
#define GLITE_WMS_X_VECTOR
#include <vector>
#endif

#ifndef GLITE_WMS_X_IOSTREAM
#define GLITE_WMS_X_IOSTREAM
#include <iostream>
#endif

#ifndef GLTIE_WMS_X_UTILITY
#define GLTIE_WMS_X_UTILITY
#include <utility>
#endif

#ifndef GLTIE_WMS_HELPER_JOBADAPTER_URL_URL_H
#include "jobadapter/url/URL.h"
#endif

#ifndef GLTIE_WMS_HELPER_JOBADAPTER_JOBWRAPPER_JOB_WRAPPER_H
#include "JobWrapper.h"
#endif

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {
namespace jobwrapper {

class MpiPbsJobWrapper : public JobWrapper
{

public:

  /**
   *  Default constructor.
   *  Initializes the object passing the parameters to the job wrapper.
   *  \param job
   *  \ingroup jobadapter
   */
  MpiPbsJobWrapper(const std::string& job);

  /**
   *  Default destructor.
   *  \ingroup jobadapter
   */
  virtual ~MpiPbsJobWrapper(void);

protected:
  virtual std::ostream& print(std::ostream& os) const;
};

} // namespace jobwrapper
} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite

#endif // GLTIE_WMS_HELPER_JOBADAPTER_JOBWRAPPER_MPI_PBS_JOB_WRAPPER_H
