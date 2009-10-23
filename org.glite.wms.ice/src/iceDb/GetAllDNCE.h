/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
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
 * Get the oldest poll_timestamp from a couple userdn,creamurl
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_GET_ALLDNCE_H
#define GLITE_WMS_ICE_GET_ALLDNCE_H

#include "AbsDbOperation.h"
#include <list>
#include <string>
#include <utility>
#include <boost/tuple/tuple.hpp>

namespace glite {
  namespace wms {
    namespace ice {
      namespace db {
	
	/**
	 *
	 */
	class GetAllDNCE : public AbsDbOperation {

	protected:
/* 	  time_t m_result; */
/* 	  const std::string m_userdn; */
/* 	  const std::string m_creamurl; */
//	  std::list< std::pair< std::string, std::string > > m_result;
	  std::list< boost::tuple< std::string, std::string, time_t > > m_result;
	  
	public:
	  GetAllDNCE( const std::string& caller ) : AbsDbOperation( caller ) { }
	  
	  virtual void execute( sqlite3* db ) throw( DbOperationException& );
	  std::list< boost::tuple< std::string, std::string, time_t > > get( void ) const { return m_result; }
	  
	};
	
      } // namespace db
    } // namespace ice
  } // namespace wms
} // namespace glite

#endif
