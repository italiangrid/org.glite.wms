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
 * ICE subscription cache
 *
 * Author: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 */
#include "subscriptionCache.h"

using namespace std;

namespace glite {
namespace wms {
namespace ice {
namespace util {

subscriptionCache* subscriptionCache::instance = NULL;
boost::recursive_mutex subscriptionCache::mutex;

subscriptionCache::subscriptionCache( ) 
{

}

//----------------------------------------------------------------------------
void subscriptionCache::insert( const std::string& s )
{
    if ( cemons.find(s) == cemons.end() ) {
        cemons.insert(s); 
    }
}

//____________________________________________________________________________
void subscriptionCache::remove(const string& s)
{
    boost::recursive_mutex::scoped_lock M( mutex );
    it = cemons.find(s);
    if( it != cemons.end() ) {
        cemons.erase( it );
    }
}

//____________________________________________________________________________
bool subscriptionCache::has(const string& s)
{
    boost::recursive_mutex::scoped_lock M( mutex );
    it = cemons.find(s);
    return ( it != cemons.end() );
}

//____________________________________________________________________________
subscriptionCache* subscriptionCache::getInstance()
{
    if (!instance)
        instance = new subscriptionCache();
    return instance;
}

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite
