// File: HelperFactory.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_HELPER_HELPERFACTORY_H
#define GLITE_WMS_HELPER_HELPERFACTORY_H

#ifndef GLITE_WMS_X_VECTOR
#define GLITE_WMS_X_VECTOR
#include <vector>
#endif
#ifndef GLITE_WMS_X_BOOST_UTILITY_HPP
#define GLITE_WMS_X_BOOST_UTILITY_HPP
#include <boost/utility.hpp>
#endif

#include <exception>

namespace glite {
namespace wms {
namespace helper {

class HelperImpl;

struct NoCreateHelperException : public std::exception
{
    const char* what() const throw () 
    { 
       return "Unknown Helper"; 
    }
};

class HelperFactory: boost::noncopyable
{
  class Impl;

  Impl* m_impl;

  static HelperFactory* s_instance;

  HelperFactory();

  typedef HelperImpl product_type;
  typedef product_type* (*product_creator_type)();

public:
  static HelperFactory* instance();
  ~HelperFactory();

public:

  bool register_helper(std::string const& id, product_creator_type creator);
  bool unregister_helper(std::string const& id);
  product_type* create_helper(std::string const& id);
  std::vector<std::string> list() const;
};

}}} // glite::wms::helper

#endif
