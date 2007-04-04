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
 * ICE CEMON URL Cache
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "DNProxyManager.h"
#include "glite/ce/cream-client-api-c/certUtil.h"

namespace iceUtil = glite::wms::ice::util;

using namespace std;

iceUtil::DNProxyManager* iceUtil::DNProxyManager::s_instance = NULL;
boost::recursive_mutex iceUtil::DNProxyManager::mutex;

//______________________________________________________________________________
iceUtil::DNProxyManager* iceUtil::DNProxyManager::getInstance() throw()
{

  boost::recursive_mutex::scoped_lock M( mutex );

  if( !s_instance ) 
    s_instance = new DNProxyManager();
  return s_instance;
}

//________________________________________________________________________
void iceUtil::DNProxyManager::setUserProxyIfLonger( const string& prx ) throw()
{ 

  string dn = glite::ce::cream_client_api::certUtil::getDN( prx );

  this->setUserProxyIfLonger( dn, prx );

}

//________________________________________________________________________
void iceUtil::DNProxyManager::setUserProxyIfLonger( const string& dn, 
							 const string& prx 
						    ) throw()
{ 

  //string dn = glite::ce::cream_client_api::certUtil::getDN( prx );

  if( m_DNProxyMap.find( dn ) == m_DNProxyMap.end() ) {
    m_DNProxyMap[ dn ] = prx;
    return;
  }

  if (prx == m_DNProxyMap[ dn ] ) return;

  time_t newT, oldT;

  try {
    newT= glite::ce::cream_client_api::certUtil::getProxyTimeLeft(prx);
  } catch(...) {
    //cout << "subscriptionManager::setUserProxyIfLonger - Cannot retrieve time for ["<<prx<<"]"<<endl;
    return;
  }
  
  try {
    oldT = glite::ce::cream_client_api::certUtil::getProxyTimeLeft( m_DNProxyMap[ dn ] );
  } catch(...) {
    // cout<< "subscriptionManager::setUserProxyIfLonger - Setting user proxy to ["
// 	<<  prx
// 	<< "] because cannot retrieve time for ["<< m_DNProxyMap[ dn ] <<"]" <<endl;
    m_DNProxyMap[ dn ] = prx;
    return;
  }

  if(newT > oldT) {
    //cout<< "subscriptionManager::setUserProxyIfLonger - Setting user proxy to ["<<prx<<"]" <<endl;
    m_DNProxyMap[ dn ] = prx;
  } else {
    // cout<< "subscriptionManager::setUserProxyIfLonger - Leaving current proxy ["<< m_DNProxyMap[ dn ] 
// 	<<"] beacuse will expire later" <<endl;
  }
}
