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

#ifndef GLITE_WMS_ICE_CHECK_DELEGATIONBYID_H
#define GLITE_WMS_ICE_CHECK_DELEGATIONBYID_H

#include "AbsDbOperation.h"
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace db {
	
	/**
	 *
	 */
	class CheckDelegationByID : public AbsDbOperation {
	
	protected:  
	  const std::string m_delegid;
	  bool m_found;

	public:
	  CheckDelegationByID( const std::string& delegid ) 
	    : AbsDbOperation(), m_delegid( delegid ), m_found( false ) {}
	  
	  virtual void execute( sqlite3* db ) throw( DbOperationException& );
	  
	  bool found( void ) const { return m_found; }
	  
	};
	
      } // namespace db
    } // namespace ice
  } // namespace wms
} // namespace glite

#endif
