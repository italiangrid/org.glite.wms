/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
* 	Authors:	Alessandro Maraschini <alessandro.maraschini@datamat.it>
* 			Marco Sottilaro <marco.sottilaro@datamat.it>
*
*/

// 	$Id$

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

