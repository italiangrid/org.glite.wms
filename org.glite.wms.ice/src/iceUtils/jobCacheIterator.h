/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE job cache Iterator
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

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
