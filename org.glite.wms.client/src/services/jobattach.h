#ifndef GLITE_WMS_CLIENT_SERVICES_JOBATTACH_H
#define GLITE_WMS_CLIENTSERVICES_JOBATTACH_H
// inheritance
#include "job.h"

namespace glite {
namespace wms{
namespace client {
namespace services {

class JobAttach: public Job  {

	public :
		JobAttach ( );

		void attach ( ) ;

};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBATTACH_H

