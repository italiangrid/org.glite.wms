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
namespace jdl {
	class Ad;		
}
namespace wms {
namespace wmproxy {
namespace eventlogger {

		
class WMPLBSelector {

	public:
		enum lbcallresult {
			SUCCESS,
			FAILURE	
		};
		
		/**
		 * Constructor
		 */
		WMPLBSelector();
			
		/**
		 * Constructor
		 */
		WMPLBSelector(std::vector<std::pair<std::string, int> > lbservers,
			std::string weightscachepath, long weightscachevaliditytime,
			bool enableservicediscovery, long servicediscoveryinfovalidity,
			const std::string &lbsdtype);
		
		/**
		 * Destructor
		 */
		 ~WMPLBSelector() throw();
		
		std::pair<std::string, int> selectLBServer();
		void updateSelectedIndexWeight(lbcallresult result);
	
	private:
		
		std::string lbsdtype;
		std::string selectedlb;
		std::vector<std::string> conflbservers;
		
		std::string weightsfile;
		std::string weightscachepath;
		long weightscachevaliditytime;
		int weightupperlimit;
		
		bool enableservicediscovery;
		long servicediscoveryinfovalidity;
		
		
		void newLBServerAd(glite::jdl::Ad &lbserverad);
		void updateLBServerAd(glite::jdl::Ad &lbserveradref,
			glite::jdl::Ad &lbserveradref);
			
		std::vector<std::string> callServiceDiscovery();
		
		void setWeightsFilePath();
		void updateWeight(glite::jdl::Ad &lbserverad, lbcallresult result);
		std::string toLBServerName(const std::string &inputstring);
		std::string toWeightsFileAttributeName(const std::string &inputstring);
		
		int generateRandomNumber(int lowerlimit, int upperlimit);
		
};

} // namespace eventlogger
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_WMPROXY_UTILITIES_WMPLBSELECTOR_H
