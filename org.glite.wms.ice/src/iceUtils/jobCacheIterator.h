
/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */
#ifndef GLITE_WMS_ICE_UTIL_JOBCACHEITERATOR_H
#define GLITE_WMS_ICE_UTIL_JOBCACHEITERATOR_H

//#include<set>
#include<string>
#include<sstream>

#include "creamJob.h"

namespace log4cpp {
    class Category;
};

namespace glite {
namespace wms {
namespace ice {
namespace util {
	
    class jobCacheIterator {

      static boost::recursive_mutex mutex;

    protected:
        bool m_valid_it;
        std::string m_grid_job_id;
        CreamJob m_theJob;
        log4cpp::Category* m_log_dev;

	
	
        /**
         * Loads the creamJob data structure from the database.  if
         * m_valid_it is true, then this method does nothing.  If
         * m_valid_it is false, then this method first tries to load a
         * creamJob object from the database. After succesfully doing
         * that, it sets m_valid_it to true. If the load from the
         * database fails, this method aborts.
         */
        void refresh() throw();

    public:
        jobCacheIterator() throw();

        /**
         * Initialize iterator with a GRID job id
         */
        jobCacheIterator( const std::string& an_id) throw();
        
        jobCacheIterator&  operator++() throw();        
        CreamJob           operator*() throw();
        CreamJob*          operator->() throw();
        bool               operator==( const jobCacheIterator& ) const throw();
        bool               operator!=( const jobCacheIterator& ) const throw();
        jobCacheIterator&  operator=( const jobCacheIterator& ) throw();   

        /**
         * Returns the grid job id this iterator points to.  If this
         * iterator does not point to a valid element (e.g., it points
         * to end() ), the empty string is returned.
         */        
        std::string get_grid_job_id( ) const { return m_grid_job_id; };
    };
    
} // util
} // ice
} // wms
} // glite

#endif
