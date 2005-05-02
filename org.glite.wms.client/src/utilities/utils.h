#ifndef GLITE_WMS_CLIENT_UTILS
#define GLITE_WMS_CLIENT_UTILS
/*
 * utils.h
 */

#include <string>
#include <vector>
#include <map>
namespace glite {
namespace wms{
namespace client {
namespace utilities {

class Utils{
public:
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
	std::pair <std::string, unsigned int>checkNs(const std::string& nsFullAddress);

	/**
	* Resolves an hostname, supposed to be an alias, into its CNAME.
	* @param hostname the hostanem to resolve.
	* @param resolved to be filled with DNS record A host entry.
	* @return whether there has been an error (true) or success (false)
	*/
	void resolveHost(const std::string& hostname, std::string& resolved);
	/**
	* Check the WMS client installation path
	* @return The string representation of the installation path
	*/
	std::string checkPrefix();

	/**
	* Check the format of the jobids
	* the format is <protocol>://<lb host>:<lb port>/<unique_string>
	* @param jobids vectors containing the list of jobids to be checked
	* @throw               in case of any format error // EXCEPTION !!!!!
	*/
	void checkJobIds(std::vector<std::string> jobids) ;
	/**
	* Get the string of the JDL read by a file
	* @param path the path of the JDL file
	*@return the JDL string
	* @throw               in case of any format error // EXCEPTION !!!!!
	*/
	std::string getJdlString (std::string path);
	void jobAdExample();
	/*
	*	extract the tokens from the input string by a given separator
	*	<field_1><separator><fields_2>....... etc
	*	@param  instr input string
	*	@sep string separator between the tokens
	*	@return a vector of strings with the extracted tokens
	*/
	const std::vector<std::string> extractFields(const std::string &instr, const std::string &sep);
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
	const long getTime(const std::string &st, const std::string &sep, const time_t &now, const unsigned int &nf = 0) ;
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
	bool isAfter (const std::string &st, const unsigned int &nf = 0) ;
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
	bool isBefore (const std::string &st, const unsigned int &nf = 0);
}; // end class definition

} // glite
} // wms
} // client
} // utilities
#endif
