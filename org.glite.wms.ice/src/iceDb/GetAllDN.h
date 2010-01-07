/* Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *    http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 *
 * Get all User Proxies
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_GET_DN_H
#define GLITE_WMS_ICE_GET_DN_H

#include "AbsDbOperation.h"
#include <list>
#include <string>

#include <boost/tuple/tuple.hpp>

namespace glite {
  namespace wms {
    namespace ice {
      namespace db {
	
	/**
	 *
	 */
	class GetAllDN : public AbsDbOperation {
	protected:
	  std::list<std::string>* m_dns;

	public:
	  GetAllDN( std::list<std::string>& target, 
		    const std::string& caller )
	    :  AbsDbOperation( caller ), m_dns( &target ) {}
	  
	  virtual void execute( sqlite3* db ) throw( DbOperationException& );
	  
	};
	
      } // namespace db
    } // namespace ice
  } // namespace wms
} // namespace glite

#endif
