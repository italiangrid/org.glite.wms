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

#ifndef GLITE_WMS_ICE_UTIL_SUBMGR_H
#define GLITE_WMS_ICE_UTIL_SUBMGR_H

#include <string>
#include <set>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/scoped_ptr.hpp>
//#include "CEDbManager.h"
#include "iceSubscription.h"
#include "iceUtils.h"

#include <utility>

#include <iostream>

#ifndef DEFAULT_PERSIST_DIR
#define DEFAULT_PERSIST_DIR "/tmp/ice_persist_dir"
#endif

//using namespace std;

namespace log4cpp {
    class Category;
};

/* namespace { */

/*   struct ltIceSub { */
/*     bool operator()( const glite::wms::ice::util::iceSubscription& s1,  */
/* 		     const glite::wms::ice::util::iceSubscription& s2) const */
/*       { */
	
/* 	if ( s1.getSubscriptionID().compare(s2.getSubscriptionID()) < 0 )  */
/* 	  return true; */
/* 	else */
/* 	  return false; */

/*       } */
/*     }; */

/* } */


namespace glite {
namespace wms {
namespace ice {
namespace util {
	
    class IceConfManager;
    
    class subscriptionProxy;
    
    //    class jobCache;
    
    class subscriptionManager {
        
        //boost::scoped_ptr< glite::wms::ice::util::CEDbManager > m_dbMgr;

      friend class iceCommandSubUpdater;
        
        std::set<std::string>                   		  m_cemonURL;
        
        std::map< std::pair<std::string, std::string> , iceSubscription, glite::wms::ice::util::ltstring > m_Subs;
        std::map< std::string, std::pair<std::string, std::string> > m_Subs_inverse;
        
        std::set<std::string>                   		  m_DN;
        std::map<std::string, std::string>      		  m_mappingCreamCemon;
        std::map<std::string, std::string>      		  m_mappingCemonDN;
        
        IceConfManager                         		         *m_conf;  
        subscriptionProxy                     		         *m_subProxy;
        static subscriptionManager                   		 *s_instance;
        log4cpp::Category                      		         *m_log_dev;
        
        static std::string 			  		  s_persist_dir;
        static bool				  		  s_recoverable_db;
        
	//        jobCache						 *m_cache;
        
        bool							  m_authz;
        bool							  m_authn;
        
        void init( void ) throw();
        
    protected:
        subscriptionManager() throw();
        ~subscriptionManager() throw() {}
        static boost::recursive_mutex  mutex;
        void getUserCEMonMapping( std::map< std::string, std::set<std::string> >& target,
                                  const bool only_active_jobs = false ) throw();
 
    public:
        
        typedef std::map< std::pair<std::string, std::string> , iceSubscription>::iterator iterator;
        typedef std::map< std::pair<std::string, std::string> , iceSubscription>::const_iterator const_iterator;
        
        static subscriptionManager* getInstance() throw();
        
        void getCEMonURL(const std::string& proxy, const std::string& creamURL, std::string& cemonURL) throw();
        
        bool getCEMonDN( const std::string& proxy, const std::string& cemonURL,std::string& DN) throw();
        
        bool hasCEMon( const std::string& cemon ) const throw()
        {
            return( m_cemonURL.find( cemon ) != m_cemonURL.end() );
        }
        
        bool isAuthorized( const std::string& DN ) const throw()
        {
	    return ( m_DN.find(DN) != m_DN.end() );
        }
        
        void insertSubscription( const std::string& userProxy,
                                 const std::string& cemonURL,
                                 const iceSubscription& S ) throw();
        
        void getCEMons( std::vector<std::string>& ) throw();
        void getCEMons( std::set<std::string>& ) throw();
        
        bool hasSubscription( const std::string& userProxy, const std::string& cemon ) const throw();
        
//        void getUserCEMonMapping( std::map< std::string, std::set<std::string> >& target, 
//                                  const bool only_active_jobs = false ) throw();
        
        void renewSubscription(const std::string& userProxy, const std::string& cemon) throw();
        void checkSubscription(const std::pair<std::string, std::set<std::string> >&) throw();
        void purgeOldSubscription( std::map<std::string, std::set<std::string> >::const_iterator it ) throw();
        
	void removeSubscriptionForDN( const std::string& userDN ) throw();

        static void setPersistDirectory(const std::string& dir) { s_persist_dir = dir; }
        static void setRecoverableDb( const bool recover ) { s_recoverable_db=recover; }
        
        int numOfSubscriptionKeys( void ) const { return m_Subs.size(); }
        
        bool getSubscriptionByDNCEMon( const std::string& dn, const std::string& cemon, iceSubscription& target) const;
        
        /**
         * @return a pair (userDN, CE) for a given subscription id
         */
        pair<std::string, std::string> getUserCEMonBySubID( const std::string& subID ) const;
        
    };
    
}
}
}
}

#endif
