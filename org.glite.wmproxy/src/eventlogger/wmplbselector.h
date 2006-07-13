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
		enum lbcallresult {
			SUCCESS,
			FAILURE	
		};
		
		enum infosource {
			NO_SOURCE,
			CONF_FILE,
			SERVICE_DISCOVERY,
		};
		
		struct lbselectioninfo {
			std::string *address;
			int port;
			int weight;
			infosource source;
		};
		
		/**
		 * Constructor
		 */
		WMPLBSelector();
			
		/**
		 * Constructor
		 */
		WMPLBSelector(std::vector<std::pair<std::string, int> > lbservers,
			bool enableservicediscovery, long servicediscoveryinfovalidity,
			const std::string &lbsdtype);
		
		/**
		 * Destructor
		 */
		 ~WMPLBSelector() throw();
		
		std::pair<std::string, int> selectLBServer();
		void updateSelectedIndexWeight(lbcallresult result);
	
	private:
		
		bool enableservicediscovery;
		long servicediscoveryinfovalidity;
		std::string lbsdtype;
		lbselectioninfo * selectedlbselectioninfo;
		std::pair<std::vector<lbselectioninfo*>*, long> lbselection;
		
		bool vectorRemovePair(std::vector<std::string> &items,
			const std::string &address, int port);
		std::vector<std::string> callServiceDiscovery();
		bool contains(const std::string &address, int port);
		void updateServiceDiscoveryLBItems(std::vector<std::string> &items);
		void addLBItem(const std::string &address, int port, infosource source);
		void addLBItems(std::vector<std::string> &items, infosource source);
		void addLBItems(std::vector<std::pair<std::string, int> >
			&items, infosource source);
		int generateRandomNumber(int lowerlimit, int upperlimit);
		
};

} // namespace eventlogger
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_WMPROXY_UTILITIES_WMPLBSELECTOR_H
