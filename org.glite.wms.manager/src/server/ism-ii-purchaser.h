// File: ism-ii-purchaser.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id: 

#ifndef _GLITE_WMS_MANAGER_SERVER_ISM_II_PURCHASER_H_
#define _GLITE_WMS_MANAGER_SERVER_ISM_II_PURCHASER_H_

namespace glite {
namespace wms {
namespace manager {
namespace server {

class ism_ii_purchaser
{
 public:
		enum exec_mode_t { _once_ = 0, _loop_ }; 
		
		ism_ii_purchaser(const std::string& hostname, const int port, const std::string& dn, const int timeout);
		void operator()();

		exec_mode_t exec_mode() const { return mode; }
		void exec_mode(exec_mode_t mode) { this -> mode = mode; }

		size_t sleep_interval() const { return this -> interval; }
	    void sleep_interval(size_t interval) { this -> interval = interval; }
		
 private:		
		std::string hostname, dn;
		int port,timeout;
		exec_mode_t mode;
		size_t interval;
};
		
} // namespace server
} // namespace manager
} // namespace wms
} // namespace glite

#endif
