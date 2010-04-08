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


#include "jobDbManager.h"
#include <iostream>

namespace iceUtil = glite::wms::ice::util;
using namespace std;

//______________________________________________________________________________
string iceUtil::jobDbManager::Iterator::operator*() const throw()
{
  //cout << "*** CALLED Iterator::operator*" << endl;	
  if(!m_cursor) return "";
  if( !m_data.get_size() ) return "";
  char* buf = (char*)malloc( m_data.get_size()+1 );
  memset( (void*)buf, 0, m_data.get_size()+1);
  memcpy((void*)buf, m_data.get_data(), m_data.get_size());
  std::string sBuf( buf );
  free(buf);
  return sBuf;
}

//______________________________________________________________________________
// This is used almost only to compare with the 'end' iterator
// no evident reason to compare with another iterator
// 
bool iceUtil::jobDbManager::Iterator::operator==(const Iterator& it) 
  const throw() 
{
  //cout << "*** CALLED Iterator::operator==" << endl;	
  if( !this->m_cursor && !it.m_cursor )
    return true;
	
  if( this->m_cursor && !it.m_cursor )
    return false;

  if( !this->m_cursor && it.m_cursor )
    return false;

  if( this->m_data.get_size() != it.m_data.get_size() )
    return false;

  if( memcmp( this->m_data.get_data(), it.m_data.get_data(), this->m_data.get_size() ) == 0 )
    return true;
  
  return false;

}

//______________________________________________________________________________
bool iceUtil::jobDbManager::Iterator::operator!=(const Iterator& it) 
  const throw() 
{
  return !(*this==it);
}

//______________________________________________________________________________
iceUtil::jobDbManager::Iterator&
iceUtil::jobDbManager::Iterator::operator++() throw()
{
  this->inc();
  return *this;
}

//______________________________________________________________________________
iceUtil::jobDbManager::Iterator&
iceUtil::jobDbManager::Iterator::operator++(const int) throw()
{
  this->inc();
  return *this;
}

//______________________________________________________________________________
iceUtil::jobDbManager::Iterator&
iceUtil::jobDbManager::Iterator::operator--() throw()
{
  this->dec();
  return *this;
}

//______________________________________________________________________________
iceUtil::jobDbManager::Iterator&
iceUtil::jobDbManager::Iterator::operator--(const int) throw()
{
  this->dec();
  return *this;
}

//______________________________________________________________________________
iceUtil::jobDbManager::Iterator&
iceUtil::jobDbManager::Iterator::operator=(const iceUtil::jobDbManager::Iterator& it) throw()
{
  //cout << "*** CALLED Iterator::operator=" << endl;

  if(this == &it) return *this;

  // let's assume the cursor is open and ready to iterate over the db's entries
  m_cursor = it.m_cursor;

  if(this->m_cursor) {
    try {
      if(m_cursor->get(&m_key, &m_data, DB_CURRENT) == DB_NOTFOUND) {
	m_cursor->close();
	m_cursor = 0;
      }
    } catch(...) {
      // FIXME
    }
  }

  return *this;
}

//______________________________________________________________________________
// COPY CTOR
//
iceUtil::jobDbManager::Iterator::Iterator( const iceUtil::jobDbManager::Iterator& it) throw()
{
  //cout << "*** CALLED COPY Iterator::CTOR" << endl;
  // let's assume the cursor is open and ready to iterate over the db's entries
  this->m_cursor = it.m_cursor;
  if(this->m_cursor) {
    try {

      if(m_cursor->get(&m_key, &m_data, DB_CURRENT) == DB_NOTFOUND) {
	m_cursor->close();
	m_cursor = 0;
      }

    } catch(...) {
      // FIXME
    }
  }
}

//______________________________________________________________________________
// CTOR with IteratorBegin argument
//
iceUtil::jobDbManager::Iterator::Iterator(const iceUtil::jobDbManager::Iterator::IteratorBegin& itb) throw() 
{
  Db* db = itb.m_db;

  db->cursor(NULL, &m_cursor, 0);
  
  //cout << "*** CALLED Iterator::CTOR" << endl;
  if( !m_cursor ) cout << "*** m_cursor is NULL!!" << endl;
  try {
    if(m_cursor->get(&m_key, &m_data, DB_FIRST) == DB_NOTFOUND) {
      m_cursor->close();
      m_cursor=NULL;
    }
  } catch(...) {

    // FIXME;

  }

}

//______________________________________________________________________________
iceUtil::jobDbManager::Iterator::~Iterator() throw()
{
}

//______________________________________________________________________________
void iceUtil::jobDbManager::Iterator::end() throw()
{
  
  try {
    if(m_cursor) m_cursor->close();
    m_cursor = 0;
  } catch(DbException& e) {
    cout << "ERROR: " << e.what() << endl;
  }

}

//______________________________________________________________________________
void iceUtil::jobDbManager::Iterator::inc( void ) throw()
{
  //cout << "*** CALLED Iterator::operator++" << endl;
  if(m_cursor) {
    try {
      if(m_cursor->get( (Dbt*)&m_key, (Dbt*)&m_data, DB_NEXT) == DB_NOTFOUND) {
	/**
	 * If not found, this iterator become the 'end' iterator
	 */
	m_cursor->close();
	m_cursor = 0;
      }
    } catch (...) {
      // FIXME
    }
  }
}

//______________________________________________________________________________
void iceUtil::jobDbManager::Iterator::dec( void ) throw()
{
  //cout << "*** CALLED Iterator::operator++" << endl;
  if(m_cursor) {
    try {
      if(m_cursor->get( (Dbt*)&m_key, (Dbt*)&m_data, DB_PREV) == DB_NOTFOUND) {

	/**
	 * If not found, this iterator become the 'end' iterator
	 */
	m_cursor->close();
	m_cursor = 0;
      }
    } catch (...) {
      // FIXME
    }
  }
}
