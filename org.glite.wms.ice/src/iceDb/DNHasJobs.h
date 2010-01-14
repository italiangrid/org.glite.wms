
#ifndef GLITE_WMS_ICE_DNHAS_JOBS_H
#define GLITE_WMS_ICE_DNHAS_JOBS_H

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
	class DNHasJobs : public AbsDbOperation {
	protected:
	  
	  std::string  m_dn;
	  bool         m_found;
	  
	public:
	  /*GetJobs( const std::list< std::pair<std::string, std::string>& clauses,
		   std::list< glite::wms::ice::util::CreamJob >& result,
		   const std::string& caller ) : AbsDbOperation( caller ), m_result( &result ), m_clause( clauses ), m_use_or( false ) {}*/
	  DNHasJobs( const std::string& dn, const std::string& caller ) : AbsDbOperation( caller ),  m_dn( dn ), m_found( false ) {}
	    
	  virtual void execute( sqlite3* db ) throw( DbOperationException& );
	  bool         found( void ) const { return m_found; }  
	    
	};
	
      } // namespace db
    } // namespace ice
  } // namespace wms
} // namespace glite

#endif
