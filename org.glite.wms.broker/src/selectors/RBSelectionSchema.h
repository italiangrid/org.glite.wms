/*
 * File: RBSelectionSchema.h
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

# ifndef GLITE_WMS_BROKER_SELECTORS_RBSELECTIONSCHEMA_H_
# define GLITE_WMS_BROKER_SELECTORS_RBSELECTIONSCHEMA_H_
# include <boost/utility.hpp>
# include <boost/thread/mutex.hpp>
# include <map>
# include <vector>
# include "glite/wms/matchmaking/matchmaker.h"

namespace matchmaking = glite::wms::matchmaking;

namespace glite {
namespace wms {		
namespace broker {
		
class RBSelectionSchema 
{
 public:
   RBSelectionSchema() {}
   virtual ~RBSelectionSchema() {}
   virtual matchmaking::match_const_iterator selectBestCE(const matchmaking::match_table_t& match_table) = 0;
};

	
class RBSelectionSchemaMap : boost::noncopyable
{
 public:
  RBSelectionSchemaMap();
  ~RBSelectionSchemaMap();
  static RBSelectionSchema*       getSchema       (const std::string& name);
  static bool                     registerSchema  (const std::string& name, RBSelectionSchema* schema);
  static RBSelectionSchema*       unregisterSchema(const std::string& name); 

 private:
  static boost::mutex m_map_access_mutex;
  static std::map<std::string, RBSelectionSchema*>* m_selection_schema_map;
};

namespace {
  static RBSelectionSchemaMap f_SelectionSchemaMapInit;
}

}; // namespace broker
}; // namespace wms
}; // namespace glite

#endif
