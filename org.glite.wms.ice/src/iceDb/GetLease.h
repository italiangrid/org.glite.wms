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
 * Get a Lease structure
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_GET_LEASE_H
#define GLITE_WMS_ICE_GET_LEASE_H

#include "AbsDbOperation.h"
#include <string>
#include <ctime>
#include "iceUtils/Lease_manager.h"

namespace glite {
  namespace wms {
    namespace ice {
      namespace db {
	
	/**
	 *
	 */
	class GetLease : public AbsDbOperation {
	protected:
	  glite::wms::ice::util::Lease_manager::Lease_t  m_result;
	  const std::string                              m_userdn, m_creamurl;
	  bool                                           m_found;

	public:
	  GetLease( const std::string& userdn, 
		    const std::string& creamurl, 
		    const std::string& caller ) 
	    : AbsDbOperation( caller ), m_userdn( userdn ), m_creamurl( creamurl ), m_result( "", "", time(0), "")
	    { }
	  
	  virtual void execute( sqlite3* db ) throw( DbOperationException& );
	  
	  /**
	   * Return the list of jobs to poll
	   */ 
	  glite::wms::ice::util::Lease_manager::Lease_t
	    get_lease( void ) const {
              return m_result;
	    }
	  
	  bool found( void ) const { return m_found; }
	  
	};
	
      } // namespace db
    } // namespace ice
  } // namespace wms
} // namespace glite

#endif
