/*
 * File: exceptions.h
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 *
 * Copyright (c) 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */ 
 
// $Id 

#ifndef _GLITE_WMS_WMPROXY_SERVER_EXCEPTIONS_H_
#define _GLITE_WMS_WMPROXY_SERVER_EXCEPTIONS_H_

#include <exception>
#include <string> 

#include "glite/wmsutils/exception/Exception.h"

#define _CANNOT_START_ERR_ 1000

namespace utilities = glite::wmsutils::exception;

namespace glite {
namespace wms {
namespace wmproxy {
namespace server {  
 
  class Exception : public utilities::Exception
  {
   protected:	
    Exception() {} 
    Exception(const std::string& s, const std::string& m, int c, const std::string& n) :
      utilities::Exception(s, m, c, n) {} 
    };
 
  class CannotStartException : public server::Exception
  {
   public: 
    CannotStartException( const std::string& message ) :
      server::Exception( std::string(""), std::string(""), _CANNOT_START_ERR_, "CannotStartException" )
    {
      this -> error_message = message;
    }
  };

} // namespace server
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif
