/*
 * File: ResourceBroker.h
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */
 
#include "glite/wms/broker/ResourceBroker.h"
#include "RBSimpleISMImpl.h"

namespace glite {
namespace wms {
namespace broker {

ResourceBroker::ResourceBroker() 
{
  changeImplementation(
    boost::shared_ptr<Impl>(new RBSimpleISMImpl())
  ); 
  m_selection_schema = boost::weak_ptr<RBSelectionSchema>(
    RBSelectionSchemaMap::getSchema("maxRankSelector")
  );
} 

void 
ResourceBroker::changeImplementation(boost::shared_ptr<Impl> impl) 
{ 
  m_impl = impl; 
}
  
void
ResourceBroker::changeSelector(const std::string& name)
{
  boost::weak_ptr<RBSelectionSchema> s(
    RBSelectionSchemaMap::getSchema(name)
  );
  if(s.lock()) m_selection_schema = s;
}


matchtable::const_iterator 
ResourceBroker::selectBestCE(matchtable const& match_table)
{
  if(RBSelectionSchemaPtr schema = m_selection_schema.lock() ) {
    return schema -> selectBestCE( match_table );
  }
  return match_table.end();
}

boost::tuple<
  boost::shared_ptr<matchtable>,
  boost::shared_ptr<filemapping>,
  boost::shared_ptr<storagemapping>
> 
ResourceBroker::findSuitableCEs(const classad::ClassAd* requestAd)
{ 
  return m_impl -> findSuitableCEs( requestAd ); 
}

}; // namespace broker
}; // namespace wms
}; // namespace glite
