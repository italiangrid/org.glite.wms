/*
 * CommandState.h
 *
 * Copyright (c) 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

#ifndef _GLITE_WMS_WMPROXY_COMMANDS_COMMANDSTATE_H_
#define _GLITE_WMS_WMPROXY_COMMANDS_COMMANDSTATE_H_
 
#include <boost/shared_ptr.hpp>
#include <queue>

namespace glite { 
namespace wms { 
namespace wmproxy { 
namespace commands {

class Command;

/**
 * This class is a superclass for all stub states in NS.
 * Each state inherits its properties and specifies new actions to be
 * done by implementing execute() method.
 *
 * @version
 * @date September 16 2002
 * @author Marco Pappalardo, Salvatore Monforte
 */ 
class CommandState
{
protected:
  /**
   * Constructor.
   */
  CommandState() {}
public:
  /** A shared pointer to a Command State. */
  typedef boost::shared_ptr<CommandState> shared_ptr;
 
  /**
   * Destructor.
   */
  virtual ~CommandState() {}

  /**
   * Return a boolean describing whether the command (the NS) 
   * must now forward a request to the Workload Manager or not.
   * @return true if NS must immediatly forward command request to WM or not.
   */
  virtual bool forwardRequest() const { return false; }

  /**
   * Return a boolean describing whether the state is a label.
   * @param label the label destination string.
   */
  virtual bool hasLabel(std::string& label) const { return false; }

  /**
   * Excecutes this command.
   * @param context the execution context.
   * @return true on success, false otherwise.
   */
  virtual bool execute(commands::Command* context) = 0;
};

typedef std::queue<CommandState::shared_ptr> state_machine_t;

}
}
}
}

#endif


