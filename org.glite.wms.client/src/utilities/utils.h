#ifndef GLITE_WMS_CLIENT_UTILS
#define GLITE_WMS_CLIENT_UTILS
/*
 * utils.h
 */

#include <string>
#include <vector>
#include <map>
#include "options_utils.h"
namespace glite {
namespace wms{
	// Wms Client Configuration class declaration
	namespace common {
		namespace configuration{
			class WMCConfiguration ;
		}
	}
namespace client {
namespace utilities {
class Utils{
public:
	Utils(Options *wmcOpt);
	/**
	* Resolves an hostname, supposed to be an alias, into its CNAME.
	* @param hostname the hostanem to resolve.
	* @param resolved to be filled with DNS record A host entry.
	* @return whether there has been an error (true) or success (false)
	*/
	bool answerYes (const std::string& question, bool defaultAnswer=false);
	/**
	* Extrapolate the list of possible LB address associated with the specified NS address index (or all when no index specified)
	* @param lbGroup a map of all the LB available. Each vector contains the LBs for a certain NS
	* @param nsNum the NetworkServer address to be referred to whn extrapolating
	*/
	std::vector<std::string> getLbs(const std::vector<std::vector<std::string> >& lbGroup,int nsNum=-1);
	/**
	* Check the LB value, the format is
	[<protocol>://]<lb host>[:<lb port>]
	*/
	std::pair <std::string, unsigned int>checkLb(const std::string& lbFullAddress);
	/* Check the NS value, the format is
	[<protocol>://]<ns host>[:<ns port>]
	*/
	std::pair <std::string, unsigned int>checkWmp(const std::string& wmpFullAddress);
	/**
	* Resolves an hostname, supposed to be an alias, into its CNAME.
	* @param hostname the hostanem to resolve.
	* @param resolved to be filled with DNS record A host entry.
	* @return whether there has been an error (true) or success (false)
	*/
	void resolveHost(const std::string& hostname, std::string& resolved);
	/**
	* Check the format of a jobid string
	* the format is <protocol>://<lb host>:<lb port>/<unique_string>
	* @param jobid jobid string to be checked
        * @return the jobid string
        *@throw WmsClientException in case of bad format
	*/
        static std::string Utils::checkJobId(std::string jobid);
	/**
	* Check the format of a list of jobids
	* the format is <protocol>://<lb host>:<lb port>/<unique_string>
        *@throw  WrongIdException  in case of bad format of one of the jobids in the list
	*/
	static void checkJobIds(std::vector<std::string> jobids) ;

	/** Exit from the process. If ncessary prompt some information
	@param exitCode command exit code, 0 in case of success, error otherwise
	*/
	void ending(unsigned int exitCode=0);

	/*const std::vector<pair<char,std::string> > extractTime(const std::string &st, const std::string &sep) ;
	const int getTime(const  std::vector<pair<char,std::string> >&vt, const time_t &now) ;
	*
	*/
	/*
	* 	check if the input time string is related to the future
	*	the formats of the input time string could be:
	*		- MM<sep>DD<sep>hh<sep>mm<sep>YYYY
	*		- MM<sep>DD<sep>hh<sep>mm
	*		- hh<sep>mm
	*	where MM=month, DD=day of the month, YYYY=year
	*		  hh=hours , min=minutes
	*	(if nf is a positve parameter check that the number of fields is equal to nf)
	*	@st the input time string
	*	@nf number of the fields expected in the input string ( 0 , no check)
	*	@return true in case of success
	*/
	static bool isAfter (const std::string &st, const unsigned int &nf = 0) ;
	/*
	* 	check if the input time string is related to the past
	*	the formats of the input time string could be:
	*		- MM<sep>DD<sep>hh<sep>mm<sep>YYYY
	*		- MM<sep>DD<sep>hh<sep>mm
	*		- hh<sep>mm
	*	where MM=month, DD=day of the month, YYYY=year
	*		  hh=hours , min=minutes
	*	(if nf is a positvecheck that the number of fields is equal to nf)
	*	@st the input time string
	*	@nf number of the fields expected in the input string ( 0 , no check)
	*	@return true in case of success
	*/
	static bool isBefore (const std::string &st, const unsigned int &nf = 0);
private:
	/**
	* Check the WMS client installation path
	* @return The string representation of the installation path and loads general info
	*/
	std::string checkPrefix(const std::string& vo);
	void checkVo();
	/** Look for possible configuration file */
	/*
	*	convert the input time string to number of seconds form 1970
	*	the formats of the input time string could be:
	*		- MM<sep>DD<sep>hh<sep>mm<sep>YYYY
	*		- MM<sep>DD<sep>hh<sep>mm
	*		- hh<sep>mm
	*	where MM=month, DD=day of the month, YYYY=year
	*		  hh=hours , min=minutes
	*	(check also if the number of fields is equal to the nf parameter)
	*	@st the input time string
	*	@sep field separator
	*	@now the current time
	*	@nf number of the fields expected in the input string
	*	@return the number of seconds
	*	@throw ---------------------------------- in case any format error in the input string
	*/
	static const long getTime(const std::string &st, const std::string &sep, const time_t &now, const unsigned int &nf = 0) ;
	/*
	*	extract the tokens from the input string by a given separator
	*	<field_1><separator><fields_2>....... etc
	*	@param  instr input string
	*	@sep string separator between the tokens
	*	@return a vector of strings with the extracted tokens
	*/
	static const std::vector<std::string> extractFields(const std::string &instr, const std::string &sep);

	// Ad configuration files:
	glite::wms::common::configuration::WMCConfiguration *wmcConf;
	// Option files:
	Options *wmcOpt;
	// General configuration inner values
	std::string prefix;

}; // end class definition

} // glite
} // wms
} // client
} // utilities
#endif
