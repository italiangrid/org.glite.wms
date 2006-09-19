/*
 * File: ResourceBroker.h
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */
 
#ifndef _GLITE_WMS_BROKER_RESOURCEBROKER_H_
#define _GLITE_WMS_BROKER_RESOURCEBROKER_H_

#include "glite/wms/matchmaking/matchmaker.h"
#include "glite/wms/brokerinfo/brokerinfo.h"
#include "glite/wms/broker/selectors/RBSelectionSchema.h"	

#include <boost/utility.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>


namespace classad
{
  class ClassAd;
}

namespace glite {
namespace wms {
namespace broker {

class ResourceBroker : boost::noncopyable
{
public: 
  class Impl;

  ResourceBroker();
  void changeImplementation(boost::shared_ptr<Impl>);
  void changeSelector(const std::string&); 
  
  matchmaking::matchtable::const_iterator 
  selectBestCE(matchmaking::matchtable const&); 
  
  boost::tuple<
    boost::shared_ptr<matchmaking::matchtable>,
    boost::shared_ptr<brokerinfo::filemapping>,
    boost::shared_ptr<brokerinfo::storagemapping>
  >
  findSuitableCEs(const classad::ClassAd*);
  
private:
  boost::shared_ptr<Impl> m_impl;
  boost::weak_ptr<RBSelectionSchema> m_selection_schema;
};

struct ResourceBroker::Impl : boost::noncopyable
{
  virtual 
  boost::tuple<
    boost::shared_ptr<matchmaking::matchtable>,
    boost::shared_ptr<brokerinfo::filemapping>,
    boost::shared_ptr<brokerinfo::storagemapping>
  >
  findSuitableCEs(const classad::ClassAd*) = 0;
  virtual ~Impl() {};
};

}; // namespace broker
}; // namespace wms
}; // namespace glite

#endif
