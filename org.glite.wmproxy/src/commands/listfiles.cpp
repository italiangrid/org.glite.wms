/*
 * File: listfiles.cpp
 * Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 * Copyright (c) 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

//  $Id$

#include "boost/filesystem/operations.hpp"
#include <iostream>


#include "listfiles.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

namespace logger = glite::wms::common::logger;

#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)

namespace fs = boost::filesystem;

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {

void list_files( const fs::path& p, std::vector<std::string>& v)
{
  if ( fs::exists(p) && fs::is_directory( p ) )
  {
    fs::directory_iterator end_iter;
    for ( fs::directory_iterator dir_itr( p );
          dir_itr != end_iter;
          ++dir_itr )
    {
      try
      {
        if ( !fs::is_directory( *dir_itr ) )
        {
#ifdef DEBUG  
	  std::cout << dir_itr->file_path() << "\n";
#endif	
	  v.push_back( dir_itr->file_path() );
        }
      }
      catch ( const std::exception & ex )
      {
        edglog( warning) << dir_itr->file_path() << " " << ex.what() << std::endl;
      }
    }
  }
}

}
}
}
}

