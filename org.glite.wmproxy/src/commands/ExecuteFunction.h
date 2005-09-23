/*
 * ExecuteFunction.h
 *
 * Copyright (c) 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 * @author Marco Pappalardo marco.pappalardo@ct.infn.it
 */

#ifndef _GLITE_WMS_WMPROXY_COMMANDS_EXECUTEFUNCTION_H_
#define _GLITE_WMS_WMPROXY_COMMANDS_EXECUTEFUNCTION_H_

#include "CommandState.h"
#include <boost/function.hpp>

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {
 
/**
 * This State receives a string command attribute over a connection. 
 *
 * @version 1.0
 * @date September 16 2002
 * @author Salvatore Monforte, Marco Pappalardo
 */
class ExecuteFunction : public CommandState 
{
 public:
  /**
   * Constructor
   * @param s the name of the attribute whose value is to be sent.
   */
  ExecuteFunction(boost::function<bool(Command*)> fn);
  
  /**
   * Describes the action this state must perform.
   * @param cmd the command the state belongs to.
   * @return whether an error code indicating whether error occurred or not.
   * @throws JDLParsingException if parsing error occurs.
   */
  bool execute (Command* cmd);

 private:
  boost::function<bool(Command*)> fn;
};

}
}
}
}

#endif 







