
#ifndef GLITE_WMS_ICE_GET_JOBS_H
#define GLITE_WMS_ICE_GET_JOBS_H

#include "AbsDbOperation.h"
#include "iceUtils/creamJob.h"
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
	class GetJobs : public AbsDbOperation {
	protected:
	  
	  std::list< glite::wms::ice::util::CreamJob >          *m_result;
	  const std::list<std::pair<std::string, std::string> >  m_clause;
	  bool 						         m_use_or;
	  
	public:
	  /*GetJobs( const std::list< std::pair<std::string, std::string>& clauses,
		   std::list< glite::wms::ice::util::CreamJob >& result,
		   const std::string& caller ) : AbsDbOperation( caller ), m_result( &result ), m_clause( clauses ), m_use_or( false ) {}*/
	  GetJobs( const std::list< std::pair<std::string, std::string> >& clauses, std::list< glite::wms::ice::util::CreamJob >& result, const std::string& caller ) : AbsDbOperation( caller ),  m_result( &result ), m_clause( clauses ), m_use_or( false ) {}
	    
	    virtual void execute( sqlite3* db ) throw( DbOperationException& );
	    
	    void use_or_clause( void ) { m_use_or = true; }
	    void use_and_clause( void ) { m_use_or = false; }
	};
	
      } // namespace db
    } // namespace ice
  } // namespace wms
} // namespace glite

#endif
