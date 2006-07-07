/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmplbselector.h
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#ifndef GLITE_WMS_WMPROXY_UTILITIES_WMPLBSELECTOR_H
#define GLITE_WMS_WMPROXY_UTILITIES_WMPLBSELECTOR_H

#include <string>
#include <vector>


namespace glite {
namespace wms {
namespace wmproxy {
namespace eventlogger {

		
class WMPLBSelector {

	public:
		struct lbselectioninfo {
			std::string *address;
			int port;
			int weight;
		};
		
		enum lbcallresult {
			SUCCESS,
			FAILURE	
		};
		
		/**
		 * Constructor
		 */
		//WMPLBSelector(const WMPLBSelector &selector);
		
		WMPLBSelector();
			
		/**
		 * Constructor
		 */
		WMPLBSelector(std::vector<std::pair<std::string, int> > lbservers,
			bool enableservicediscovery, long servicediscoveryinfovaliditytime,
			const std::string &lbsdtype);
		
		/**
		 * Destructor
		 */
		virtual ~WMPLBSelector() throw();
		
		std::pair<std::string, int> selectLBServer();
		void updateSelectedIndexWeight(lbcallresult result);
	
	private:
		
		bool enableservicediscovery;
		long servicediscoveryinfovaliditytime;
		std::string lbsdtype;
		lbselectioninfo * selectedlbselectioninfo;
		std::pair<std::vector<lbselectioninfo*>*, long> lbselection;
		
		bool contains(const std::string &lbitem);
		std::vector<std::string> callServiceDiscovery();
		int generateRandomNumber(int lowerlimit, int upperlimit);
		
};

} // namespace eventlogger
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_WMPROXY_UTILITIES_WMPLBSELECTOR_H
