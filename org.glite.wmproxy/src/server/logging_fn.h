/*
 * File: logging_fn.h
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 * Copyright (c) 2005 EGEE.
 */

// $Id

namespace glite {
namespace wms {
namespace wmproxy {
namespace server {


extern "C++"				
{
  bool LogEnqueuedJob(edg_wll_Context, std::string, std::string, 
		      std::string, const char*, bool, const char*, bool, bool);
  bool LogEnqueuedJob(edg_wll_Context, const char*, bool, const char*, bool, bool);
}

} // namespace server
} // namespace wmproxy
} // namespace wms
} // namespace glite
