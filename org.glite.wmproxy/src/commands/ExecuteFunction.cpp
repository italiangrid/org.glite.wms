/*
 * ExecuteFunction.cpp
 *
 * Copyright (c) 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 * @author Marco Pappalardo marco.pappalardo@ct.infn.it
 */

#include "Command.h"
#include "ExecuteFunction.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {

ExecuteFunction::ExecuteFunction(boost::function<bool(Command*)> f)
{
 fn = f;
} 

bool ExecuteFunction::execute(Command* cmd)
{
  return fn(cmd);
}

}
}
}
}



