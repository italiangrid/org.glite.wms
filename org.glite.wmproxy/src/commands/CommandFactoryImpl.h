/*
 * CommandFactoryImpl.h
 * 
 * Copyright (c) 2001, 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */
 
#ifndef _GLITE_WMS_WMPROXY_COMMANDS_COMMANDFACTORYIMPL_H_
#define _GLITE_WMS_WMPROXY_COMMANDS_COMMANDFACTORYIMPL_H_

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {
 
class Command;

/**
 * The command-factory definition.
 *
 * @version
 * @date September 16 2002
 * @author Salvatore Monforte
 */
class CommandFactoryImpl
{
public:
  /**
   * Creates a new command of the specified type.
   * @return a pointer to a new command.
   */
  virtual Command* create(const std::string& name) = 0;
  /**
   * Destructor.
   */
  virtual ~CommandFactoryImpl() {}
};

}
}
}
}
 
#endif
