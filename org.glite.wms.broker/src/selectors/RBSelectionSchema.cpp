// File: RBSelectionSchema.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$ 

#include "RBSelectionSchema.h"
#include "maxRankSelector.h"
#include "stochasticRankSelector.h"

#include <string>
#include <map>

using namespace std;

namespace glite {
namespace wms {
namespace broker {

namespace 
{
  static unsigned int f_selection_map_instance_count;	
}
	
  boost::mutex                               RBSelectionSchemaMap::m_map_access_mutex;
  // leave as pointer to std::map
  std::map<std::string, RBSelectionSchema*>* RBSelectionSchemaMap::m_selection_schema_map;
  
  RBSelectionSchemaMap::RBSelectionSchemaMap()
  {
    boost::mutex::scoped_lock lock(m_map_access_mutex);
    if( !f_selection_map_instance_count++ ) {   
      // register built-in schemas
      m_selection_schema_map = new std::map<std::string, RBSelectionSchema*>(); 
      (*m_selection_schema_map)["maxRankSelector"]        = new maxRankSelector();
      (*m_selection_schema_map)["stochasticRankSelector"] = new stochasticRankSelector();
    }
  }
 
RBSelectionSchemaMap::~RBSelectionSchemaMap()
{
  boost::mutex::scoped_lock lock(m_map_access_mutex);
  if( !--f_selection_map_instance_count ) {
	  // unregister all schemas
  	   while( !m_selection_schema_map->empty() ) {
		delete m_selection_schema_map->begin() -> second;
		m_selection_schema_map->erase( m_selection_schema_map->begin() );
	    }
	   delete m_selection_schema_map;
  }
}

bool RBSelectionSchemaMap::registerSchema(const std::string& name, RBSelectionSchema* schema)
{
  boost::mutex::scoped_lock lock(m_map_access_mutex);
  if( m_selection_schema_map->find(name) != m_selection_schema_map->end() ) return false; 
  (*m_selection_schema_map)[name] = schema;
  return true;  
}

RBSelectionSchema* RBSelectionSchemaMap::unregisterSchema(const std::string& name) 
{
  // Since the schema will be unregistered its memory disposing 
  // is no longer responsability of the RBSelectionSchemaMap
  boost::mutex::scoped_lock lock(m_map_access_mutex);
  if( m_selection_schema_map->find(name) == m_selection_schema_map->end() ) return 0;
  RBSelectionSchema* schema = (*m_selection_schema_map)[name];
  m_selection_schema_map->erase(name); 
  return schema;  
}

RBSelectionSchema* RBSelectionSchemaMap::getSchema(const std::string& name)
{
 boost::mutex::scoped_lock lock(m_map_access_mutex);
  if( m_selection_schema_map->find(name) == m_selection_schema_map->end() ) return 0;
  return (*m_selection_schema_map)[name];
}

} // namespace broker
} // namespace wms
} // namespace glite
