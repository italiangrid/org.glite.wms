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
 * Event status poller
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

// ICE includes
#include "ice-core.h"
#include "eventStatusPoller.h"
#include "iceCommandStatusPoller.h"
#include "iceCommandEventQuery.h"
#include "iceDb/GetFields.h"
#include "iceDb/Transaction.h"

// other glite includes
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/functional.hpp>

#include <algorithm>
#include <cstdlib>

namespace cream_api=glite::ce::cream_client_api;
using namespace glite::wms::ice::util;
using namespace std;

//____________________________________________________________________________
// class Randint {
//   unsigned long randx;
// public:
//   Randint(long s = 0) { randx = s; }
//   void seed(long s) { randx = s; }
//   long abs( long x ) { return x&0x7fffffff; }
//   static double max( ) { return 2147483648.0; }
//   long draw( ) { return randx = randx*1103515245 + 12345; }
//   double fdraw( ) { return abs( draw() )/max(); }
//   long operator()() { return abs( draw() ); }
// };

// //____________________________________________________________________________
// class Urand : public Randint {
//   long n;
// public:
//   Urand( long nn ) { n=nn; }
//   long operator()() {long r = n*fdraw(); return (r==n) ? n-1:r; } 
// };

//____________________________________________________________________________
eventStatusPoller::eventStatusPoller( glite::wms::ice::Ice* manager, int d )
    : iceThread( "event status poller" ),
      m_delay( d ),
      m_iceManager( manager ),
      m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
      m_threadPool( manager->get_ice_commands_pool() )//,
      //m_real_poller( m_iceManager )
{

}

//____________________________________________________________________________
eventStatusPoller::~eventStatusPoller()
{

}

//____________________________________________________________________________
void eventStatusPoller::body( void )
{
  //iceCommandStatusPoller real_poller( m_iceManager );

    while( !isStopped() ) {

        /**
         * We don't use boost::thread::sleep because right now
         * (18/11/2005) the documentation says it will be replaced by
         * a more robust mechanism in the future.
         */
      if(m_delay<=10) 
        sleep( m_delay );
      else {

	for(int i=0; i<=m_delay; i++) {
	  if( isStopped() ) return;
	  sleep(1);
	}

      }

        // Thread wakes up

	

        CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << "eventStatusPoller::body() - New iteration"
                         );


	list< vector< string > > result;
	{
	  /*
	    SELECT DISTINCT (userdn, creamurl) from jobs;
	  */
	  list< string > fields;
	  fields.push_back( "userdn" );
	  fields.push_back( "creamurl" );
	  
	  db::GetFields getter( fields, list< pair< string, string > >(), result, "eventStatusPoller::body", true/*=DISTINCT*/ );
	  db::Transaction tnx(false, false);
	  tnx.execute( &getter );
	  //result = getter.get_values();
	}

// 	Urand r(52);
// 	list< vector< string > >::const_iterator it;
// 	for( it=result.begin(); it!=result.end(); ++it )
// 	  {
// 	    if( it->at(0).empty() || it->at(1).empty() )
// 	      continue;

// 	    DNs.push_back( it->at(0) );
// 	    CEs.push_back( it->at(1) );

// 	    mapDNCE[ it->at(0) ].push_back( it->at(1) );
// 	  }

// 	random_shuffle( DNs.begin(), DNs.end(), r );
// 	random_shuffle( CEs.begin(), CEs.end(), r );


	/**
	   Organize on distinct DNs
	   The map is DN -> ArrayOf(CEs)
	*/
	list< vector< string > >::const_iterator it;
	map< string, list<string> > mapDNCE;
	for(it=result.begin(); it != result.end(); ++it )
	  {
	    if( it->at(0).empty() || it->at(1).empty() )
	      continue;

	    mapDNCE[ it->at(0) ].push_back( it->at(1) );
	  }
	
        //m_pool->add_request( new iceCommandStatusPoller( m_iceManager ) );
	//iceCommandStatusPoller( m_iceManager ).execute();
	//A	m_real_poller.execute();

	CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << "eventStatusPoller::body() - There're ["
			<< result.size() << "] distinct couple(s) of DN,CE to "
			<< " check for job's events..."
			);


	result.clear();
	map< string, list<string> >::const_iterator dnit;
	for(dnit = mapDNCE.begin(); dnit != mapDNCE.end(); ++dnit )
	  {
	    /**
	       dnit is a couple DN,CE1...CEN. Will ask for Events
	       of DN to multiple CE in parallel
	    */
	    list<string>::const_iterator ceit;
	    for( ceit = mapDNCE[ dnit->first ].begin(); 
		 ceit != mapDNCE[ dnit->first ].end();
		 ++ceit )
	      {
 		while(m_threadPool->get_command_count() < result.size() )
		  sleep(1);
 		  m_threadPool->add_request( new iceCommandEventQuery( m_iceManager, dnit->first , *ceit ) );
	      }
	  }

// 	list< vector< string > >::const_iterator it = result.begin();
// 	for( ; it!=result.end(); ++it )
// 	  {
// 	    //	    cerr << it->at(0) << " - " << it->at(1) << endl;
// 	    if(m_threadPool->get_command_count() < result.size() )
// 	      m_threadPool->add_request( new iceCommandEventQuery( m_iceManager, *it ) );
// 	  }
	//	iceCommandEventQuery( m_iceManager ).execute();
    }
}
