// File: WMFactory.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_COMMON_WMFACTORY_H
#define GLITE_WMS_MANAGER_COMMON_WMFACTORY_H

#ifndef GLITE_WMS_X_STRING
#define GLITE_WMS_X_STRING
#include <string>
#endif
#ifndef BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef BOOST_UTILITY_HPP
#include <boost/utility.hpp>
#endif

#include <exception>

namespace glite {
namespace wms {
namespace manager {
namespace common {

class WMImpl;

struct NoCreateWMException: public std::exception
{
  const char* what() const throw () 
  { 
    return "Unknown WorkloadManager"; 
  }
};
 
class WMFactory: boost::noncopyable
{
  class Impl;
  
  boost::scoped_ptr<Impl> m_impl;
  
  static WMFactory* s_instance;
  
  WMFactory();
  
  typedef WMImpl product_type;
  typedef product_type* (*product_creator_type)();
  
 public:
  static WMFactory* instance();
  ~WMFactory();
  
public:
  
  typedef std::string wm_type;
  
  bool register_wm(wm_type const& id, product_creator_type creator);
  bool unregister_wm(wm_type const& id);
  product_type* create_wm(wm_type const& id);
};

}}}} // // glite::wms::manager::common


#endif // GLITE_WMS_MANAGER_COMMON_WMFACTORY_H
