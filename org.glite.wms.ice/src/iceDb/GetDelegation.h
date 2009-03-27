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

#ifndef GLITE_WMS_ICE_GET_DELEGATION_H
#define GLITE_WMS_ICE_GET_DELEGATION_H

#include "AbsDbOperation.h"
#include <string>
#include "iceUtils/Delegation_manager.h"

namespace glite {
  namespace wms {
    namespace ice {
      namespace db {
	
	/**
	 *
	 */
	class GetDelegation : public AbsDbOperation {
	protected:
	  glite::wms::ice::util::Delegation_manager::table_entry m_result;
	  const std::string m_digest, m_creamurl;
	  bool m_found;

	public:
	  GetDelegation( const std::string& digest, 
			 const std::string& creamurl ) 
	    : AbsDbOperation(), m_digest( digest ), m_creamurl( creamurl ) 
	    { }
	  
	  virtual void execute( sqlite3* db ) throw( DbOperationException& );
	  
	  /**
	   * Return the list of jobs to poll
	   */ 
	  glite::wms::ice::util::Delegation_manager::table_entry
	    get_delegation( void ) const {
              return m_result;
	    }
	  
	  bool found( void ) const { return m_found; }
	  
	};
	
      } // namespace db
    } // namespace ice
  } // namespace wms
} // namespace glite

#endif
