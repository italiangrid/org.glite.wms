/**
 * @file ForwardRequest.cpp
 * Defines ForwardRequest class taking care of
 * request forwarding through FileList.
 * 
 * @author Marco Pappalardo marco.pappalardo@ct.infn.it
 */ 

#include "Command.h"
#include "ForwardRequest.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {

ForwardRequest::ForwardRequest()
{
} 

bool ForwardRequest::execute(Command* cmd)
{
  return true;
}

bool ForwardRequest::forwardRequest() const
{
  return true;
}

}
}
}
}


