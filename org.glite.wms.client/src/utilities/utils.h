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
[<protocol>://]<lb host>:<lb port>
*/
std::pair <std::string, unsigned int>checkLb(const std::string& lbFullAddress);
std::pair <std::string, unsigned int>checkNs(const std::string& nsFullAddress);

/**
 * Resolves an hostname, supposed to be an alias, into its CNAME.
 * @param hostname the hostanem to resolve.
 * @param resolved to be filled with DNS record A host entry.
 * @return whether there has been an error (true) or success (false)
 */
bool resolveHost(const std::string& hostname, std::string& resolved);
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
} // glite
} // wms
} // client
} // utilities
#endif
