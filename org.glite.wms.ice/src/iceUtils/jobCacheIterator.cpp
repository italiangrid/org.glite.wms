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

#include "jobCacheIterator.h"
#include "jobCache.h"

//#include<sstream>

#include <boost/archive/text_iarchive.hpp>

using namespace glite::wms::ice::util;
using namespace std;

jobCache* jobCacheIterator::s_cache( 0 );

//____________________________________________________________________
bool jobCacheIterator::operator==( const jobCacheIterator& anIt ) 
  const throw()
{
  return ( m_it == anIt.m_it );
}

//____________________________________________________________________
bool jobCacheIterator::operator==( const set<string>::iterator anIt ) 
  const throw()
{
  return ( m_it == anIt );
}

//____________________________________________________________________
bool jobCacheIterator::operator!=( const jobCacheIterator& anIt ) 
  const throw()
{
  return !( m_it == anIt.m_it );
}

//____________________________________________________________________
bool jobCacheIterator::operator!=( const set<string>::iterator anIt ) 
  const throw()
{
  return !( m_it == anIt );
}

//____________________________________________________________________
jobCacheIterator&
jobCacheIterator::operator=( const set<string>::iterator anIt ) 
  throw()
{
  m_it = anIt;
  m_valid_it = false;
  return *this;  
}

//____________________________________________________________________
jobCacheIterator&
jobCacheIterator::operator=( const jobCacheIterator& anIt ) 
  throw()
{
  m_it = anIt.m_it;
  m_valid_it = false;
  return *this;
}

//___________________________________________________________________
CreamJob*
jobCacheIterator::operator->() throw()
{

  if(*this == s_cache->end() )
    abort();

  if( m_valid_it ) return &m_theJob;
  
  m_theJob = CreamJob();
  
  try {
    istringstream is;
    is.str( s_cache->getDbManager()->getByGid( *m_it ) );
    boost::archive::text_iarchive ia(is);
    ia >> m_theJob;
    m_valid_it = true;
  } catch(JobDbException& ex) {
    ;
  }
  return &m_theJob;
}

//____________________________________________________________________
CreamJob&
jobCacheIterator::operator*() throw()
{
  //string serjob( s_cache->getDbManager()->getByGid( *m_it ) );
  //CreamJob cj;
  //istringstream tmpOs;
  
  if(*this == s_cache->end() )
    abort();

  if( m_valid_it ) return m_theJob;

  //CreamJob aJob;

  m_theJob = CreamJob();

  try {
    istringstream is;
    //cout << "**** DEBUG jobCacheIterator::operator*() - Looking for GID [" << *m_it << "]" << endl;
    is.str( s_cache->getDbManager()->getByGid( *m_it ) );
    boost::archive::text_iarchive ia(is);
    ia >> m_theJob;
    m_valid_it = true;
  } catch(JobDbException& ex) {
    ;
  }
  
  return m_theJob;
}
