/*
 * File: CommandFactory.h
 *
 * Copyright (c) 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

#ifndef _GLITE_WMS_WMPROXY_COMMANDS_COMMANDFACTORY_H_
#define _GLITE_WMS_WMPROXY_COMMANDS_COMMANDFACTORY_H_

#include <boost/scoped_ptr.hpp>
#include "Command.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {

class CommandFactoryImpl;

/**
 * The command factory class template.
 * 
 * @version 1.0
 * @date September 16 2002
 * @author Salvatore Monforte, Marco Pappalardo.
 */
template<class implementation>
class CommandFactory 
{
 public:
   CommandFactory() : impl( new implementation ) {}
  
  /**
   * Creates a Command by name.
   * @return a pointer to new Command.
   */
  Command* create(const std::string& name, const std::vector<std::string>& param){ return impl -> create( name, param ); }
  
private:
  boost::scoped_ptr<CommandFactoryImpl> impl;
};

}
}
}
}

#endif
