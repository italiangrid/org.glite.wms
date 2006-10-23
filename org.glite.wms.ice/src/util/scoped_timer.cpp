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
 * Scoped timer for ICE 
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
#include "scoped_timer.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "boost/format.hpp"

using namespace glite::wms::ice::util;

scoped_timer::scoped_timer( const std::string& name ) :
    m_name( name ),
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
{
    gettimeofday( &m_start_time, 0 );
}

scoped_timer::~scoped_timer( )
{
    struct timeval m_end_time;
    gettimeofday( &m_end_time, 0 );
    
    double tstart = (double)m_start_time.tv_sec + (double)m_start_time.tv_usec/(double)1000000.0;

    double tend = (double)m_end_time.tv_sec + (double)m_end_time.tv_usec/(double)1000000.0;

    double elapsed = tend - tstart;
    
    CREAM_SAFE_LOG(m_log_dev->infoStream()
                   << boost::format( "scoped_timer %s %f %f %f" ) % m_name % tstart % tend % elapsed
                   << log4cpp::CategoryStream::ENDLINE);
    
};

