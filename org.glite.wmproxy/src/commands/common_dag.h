/*
 * common_dag.h
 *
 * Copyright (c) 2001, 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

#ifndef _GLITE_WMS_WMPROXY_COMMANDS_COMMON_DAG_H_
#define _GLITE_WMS_WMPROXY_COMMANDS_COMMON_DAG_H_

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {

    struct computeFileSize
    {
      computeFileSize(off_t& offt, bool& r) : size(offt), result(r)
      {
        result = true;
      }

      void operator() (const std::string& file)
      {
        int fd = -1;
        fd = open(file.c_str(), O_RDONLY);
        if (fd != -1) {
          struct stat buf;
          if ( !fstat(fd, &buf) ) {
            size += buf.st_size;
          }
        } else {
#if DEBUG
	  //must define the namespace logger
          logger::threadsafe::edglog << "Job: " << JobId << ". File " << file.c_str() << " not found." << std::end\
	    l;
#endif
          result = false;
        }
      }
      off_t& size;
      bool& result;
    };


} // namespace commands
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif
