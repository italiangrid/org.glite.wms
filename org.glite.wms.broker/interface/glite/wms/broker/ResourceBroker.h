/*
 * File: ResourceBroker.h
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */
 
#ifndef _GLITE_WMS_BROKER_RESOURCEBROKER_H_
#define _GLITE_WMS_BROKER_RESOURCEBROKER_H_

#include "glite/wms/matchmaking/matchmaker.h"
#include "glite/wms/broker/selectors/RBSelectionSchema.h"	

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

namespace classad
{
  class ClassAd;
}

namespace glite {
namespace wms {
namespace broker {

class ResourceBrokerImpl : boost::noncopyable
{
 public:
  ResourceBrokerImpl() {}
  // TODO: no op if requestAd=0
  virtual matchmaking::match_table_t* findSuitableCEs(const classad::ClassAd* requestAd) = 0;
  virtual ~ResourceBrokerImpl() {}
};
 
class ResourceBroker : boost::noncopyable
{ 
public:
  ResourceBroker(ResourceBrokerImpl* impl) : m_impl( impl ), 
    m_selection_schema( RBSelectionSchemaMap::getSchema("maxRankSelector") ) {} 
  ~ResourceBroker() {}
 
  void changeImplementation(ResourceBrokerImpl* impl) { m_impl.reset(impl); }
  
  RBSelectionSchema* changeSelector(const std::string& name) 
  {
    RBSelectionSchema* old_schema = m_selection_schema;
    if((m_selection_schema = RBSelectionSchemaMap::getSchema(name)) ) return old_schema;
    m_selection_schema = old_schema;
    return 0; 
  }

  matchmaking::match_const_iterator selectBestCE(const matchmaking::match_table_t& match_table) 
  {
    return m_selection_schema -> selectBestCE( match_table );
  }

  matchmaking::match_table_t* findSuitableCEs(const classad::ClassAd* requestAd)
  { 
    return m_impl -> findSuitableCEs( requestAd ); 
  }
  
private:
  boost::scoped_ptr<ResourceBrokerImpl> m_impl;
  RBSelectionSchema* m_selection_schema;
};

}; // namespace broker
}; // namespace wms
}; // namespace glite

#endif
