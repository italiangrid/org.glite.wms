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

#ifndef GLITE_WMS_ICE_GET_FIELDSCOUNT_H
#define GLITE_WMS_ICE_GET_FIELDSCOUNT_H

#include "AbsDbOperation.h"
#include <list>
#include <utility>
#include <string>

namespace glite {
  namespace wms {
    namespace ice {
      namespace db {
	
	/**
	 *
	 */
	class GetFieldsCount : public AbsDbOperation {
	protected:
	  //std::list< std::list<std::string> > m_result;
	  const std::list<std::string> m_fields_to_retrieve;
	  const std::list<std::pair<std::string, std::string> > m_clause;
	  int m_fields_count;

	public:
	  GetFieldsCount( const std::list<std::string> fields_to_retrieve, 
			  const std::list<std::pair<std::string, std::string> > clause, 
			  const std::string& caller ) 
	    : AbsDbOperation( caller ),
	    m_fields_to_retrieve( fields_to_retrieve ),
	    m_clause( clause ),
	    m_fields_count( 0 ) {}
	    
	  
	  virtual void execute( sqlite3* db ) throw( DbOperationException& );
	  
	  /**
	   * Return the list of jobs to poll
	   */ 
	  int get_count( void ) const {
            return m_fields_count;
	  }
	  
	};
	
      } // namespace db
    } // namespace ice
  } // namespace wms
} // namespace glite

#endif
