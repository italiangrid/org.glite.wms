/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$

#ifndef GLITE_WMS_CLIENT_UTILS
#define GLITE_WMS_CLIENT_UTILS
/**
 * utils.h
 */

#include <string>
#include <vector>
#include <map>
#include "excman.h"
#include "logman.h"
#include "options_utils.h"
#include "adutils.h"

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
	/**
        * Default constructor
        */
	Utils(Options *wmcOpts);
        /**
        * Default destructor
        */
	~Utils( );
	/**
	* Type of Menu
	*/
	enum WmcMenu {
		MENU_JOBID,
		MENU_CE,
		MENU_SINGLECE,
		MENU_SINGLEJOBID,
		MENU_FILE,
		MENU_SINGLEFILE,
	};
	enum WmcInputType{
		JOBID_TYPE,
		CE_TYPE
	};
	/**
	* Prompt the users (if --noint option is not active) a question with Y/N answer
	* @param question the question to be prompted to the user
	* @param defaultAnswer the default answer if noint is active or enter is typed (true= "YES")
	* @param defaultValue default value returned in case of interaction is not allowed (--noint option)
	* @return the answer value or the default value in case of interaction is not allowed
	*/
	bool answerYes (const std::string& question, bool defaultAnswer=false, bool defaultValue=false);
	/**
	* Prompt the users if they want to overwrite a file if it already exists
	* @param path the pathname of the file
	* @return TRUE if the file can be saved to the indicated path (if it doesn't exists or overwriting is allowed), FALSE otherwise
	*/
	bool askForFileOverwriting(const std::string &path);
	/**
	* Asks users to chose either all or some items in a list; the question is printed on the std-output;
        * users insert their reply on the the std-input
	* @param jobids a vector contatining the list of jobids in which perform the choice
	* @param type of menu
	* @return the list of chosen items
	*/
        std::vector<std::string> askMenu(const std::vector<std::string> &items, const enum WmcMenu &type);
	/**
	* Extrapolate the list of possible LB address associated with the specified NS address index (or all when no index specified)
	* @param lbGroup a map of all the LB available. Each vector contains the LBs for a certain NS
	* @param nsNum the NetworkServer address to be referred to whn extrapolating
	*/
	std::vector<std::string> getLbs(const std::vector<std::vector<std::string> >& lbGroup,int nsNum=-1);
        /**
        *  generates a random number included in the range[min,max]
        * @param min the minimum value of the range
        * @param max the maxium value of the range
        * @return the generated number
        */
       static const int getRandom (const unsigned int &max );
	/**
	*  Retrieves the client installation path
        * @return the installation path string representation
        */
        std::string getPrefix();
	/**
	* Gets the extension used for archive files
	* @return the file extension string
	*/
	static std::string getArchiveExtension( ) ;
	/**
	* Gets the extension used for compressed files
	* @return the file extension string
	*/
	static std::string getZipExtension( ) ;
	/**
	* Extracts from the input string the name of the archive filename according to
	* the default archive extension
	* @return the name of the archive file or an empty string if the input name is unvalid
	*
	*/
	static std::string getArchiveFilename (const std::string file);
        /**
	* Extrapolate the list of possible WMProxy addresses
	* @return  a vector of strings containg the list of addresses
	*/
	std::vector<std::string> getWmps( );
        /**
        * Gets the ErrorStorage pathname
        * @return the pathname string
        */
        const std::string getErrorStorage( );
        /**
        * Gets the OuputStorage pathname
        * @return the pathname string
        */
        std::string getOutputStorage( );
	/**
        * Gets the log file pathname
        * @return the pathname string
        */
        std::string* getLogFileName ( );
	/**
        * Gets the conf pathname
        * @return the pathname string
        */
        glite::wms::common::configuration::WMCConfiguration* getConf(){return wmcConf;}
        /**
	* Check the LB value, the format is
	[<protocol>://]<lb host>[:<lb port>]
	*/
	std::pair <std::string, unsigned int>checkLb(const std::string& lbFullAddress);
	/** Check the NS value, the format is
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
        static std::string checkJobId(std::string jobid);
	/**
	* Check the format of a list of jobids
	* the format is <protocol>://<lb host>:<lb port>/<unique_string>
        * In case the list contains some bad id's, question whether the user want to continue is asked
        *@param jobids the list of input jobid's (at the end of execution, bad format id's has been removed);
	*@return the list of the rights jobid's
	*/
	std::vector<std::string>  checkJobIds(std::vector<std::string> &jobids);
	/**
	* Check the format of a jobid string
	* the format is <protocol>://<lb host>:<lb port>/<unique_string>
	* @param jobid jobid string to be checked
        * @return the jobid string
        *@throw WmsClientException in case of bad format
	*/
        static void checkResource(const std::string& resource);
	/**
	* Check the format of a list of jobids
	* the format is <protocol>://<lb host>:<lb port>/<unique_string>
        * In case the list contains some bad id's, question whether the user want to continue is asked
        *@param jobids the list of input jobid's (at the end of execution, bad format id's has been removed);
	*@return the list of the rights jobid's
	*/
	std::vector<std::string> checkResources(std::vector<std::string> &resources);
	/**
	* Get the unique string of the input JobId
	* @param jobid the input jobid
        * @return the unique string
        *@throw WmsClientException in case of bad format
	*/
	static std::string getUnique(std::string jobid);
        /**
	* Builds a char stripe
	* @param len the length of the stripe to be built
        * @param ch the character to be used for the stripe
        * @param msg the message that can be put in the middle
        * @return the built stripe
	*/
        const std::string getStripe (const int &len, const std::string &ch, const std::string &msg = "");
	/** Exit from the process. If ncessary prompt some information
	@param exitCode command exit code, 0 in case of success, error otherwise
	*/
	static void ending(unsigned int exitCode=0);
	/**
	*  convert the input time string to the number of seconds since 1st of January 1970
        * and if a type of option is specified, check if it is a valid value for that option
        * (is later or earlier)
	* the formats of the input time string could be:
	*	- MM<sep>DD<sep>hh<sep>mm<sep>YYYY
	*	- MM<sep>DD<sep>hh<sep>mm
	*	- hh<sep>mm
	* where MM=month, DD=day of the month, YYYY=year hh=hours , min=minutes
	* @st the input time string
	* @opts type of time option
	* @return the number of seconds since 1st of January 1970
	*/
	static const long checkTime ( const std::string &st, int &days,  int &hours, int &minutes, const Options::TimeOpts &opt = Options::TIME_NO_OPT);
	/**
	* generate a unique string
        * @return the generated string
	*/
        static std::string* getUniqueString ( ) ;
        /**
        * Gets the delegation ID string: if the --delegation option has been specified its value arguments
        * is returned; otherwise the autogeneration is performed
	* @return the point to the delegation ID string
        */
        std::string* getDelegationId ( );
        /**
        * Performs the operations to delegate a user proxy to the endpoint
        * @param cfg the configuration context object
        * @param id the delegation identifier string to be used for the delegation
        * @return the pointer to the string containing the EndPoint URL
        */
        const std::string delegateProxy(glite::wms::wmproxyapi::ConfigContext *cfg, const std::string &id);
        /**
        * checks if a pathname is a valid file
        *@param pathname the pathname that has to be checked
        *@return true if the pathname is a valid file (false, otherwise)
        */
        static const bool isFile (const std::string &pathname);
        /**
        * checks if a pathname is a valid directory
        *@param pathname the pathname that has to be checked
        *@return true if the pathname is a valid directory (false, otherwise)
        */
        static const bool isDirectory (const std::string &pathname);
        /**
        * gets the absolute path of the file
        * @param file the file
        * @return the absolute pathname
        */
        static const std::string getAbsolutePath(const std::string &file );
	/**
        *  Gets the size of a file (in bytes)
        * @param path the pathanme of the file
        * @return the number of bytes in the file
        * @throw WmsClientException if the file doesn't exist or in case it is not possible to open the file
        */
        static int getFileSize (const std::string &path) ;
	/**
	* Removes a file
	* @param file the pathname of the file to be removed
	*@throw WmsClientException if an error occurred during the operation
	*/
	static void removeFile(const std::string &file) ;
	/**
        * Reads <nbytes> bytes from a file; if nbytes is set to the default value (=0),
        * the entire text of the file is read
        * @param path the pathanme of the file
        * @return the text read from the file
        * @throw WmsClientException if the file doesn't exist or in case of any error occured during the reading operations
        */
        std::string* fromFile (const std::string &path) ;
	/**
        * Saves the input message into a file
        * @param path the pathname of the file
	* @param msg the message to be saved
        * @param interactive if the flag is true, question if it can be replaced is asked in case the file already exists
        * @return 0 in case of success (-1 otherwise)
        */
         int toFile (const std::string &path, const std::string &msg, const bool &append=false);
	/**
        * Saves the identifier of a submitted job in the specified file
        * @param path the pathname of the file
	* @param jobid the job identifier string
        * @return 0 if the jobid has been successfully saved (-1 otherwise)
        */
         const int saveJobIdToFile (const std::string &path, const std::string jobid);
	/**
        * Saves a list of items in the specified file
        * @param path the pathname of the file
	* @param list the vector containing the list to be saved
        * @return 0 if the list has been successfully saved (-1 otherwise)
        */
	 const int Utils::saveListToFile (const std::string &path, const std::vector<std::string> &list);
        /**
        * Reads a file and gets the list of items contained in it
        * @param the pathname of the file to be read
        * @return the vector containg the list of items
        * @throw WmsClientException if the file doesn't exist or in case of any error occured during the reading operations
        */
        std::vector<std::string> getItemsFromFile (const std::string &path);
        /**
	* Adds the star (*) wildcard to the input pathname
        * @param path the input pathname
        * @return the modified pathname with the wildcard
	*/
	static const std::string addStarWildCard2Path(std::string& path);
 	/**
 	* Removes '/' characters at the end of the of the input pathname
        * @param fpath the input pathname
        * @return the normalized pathname
 	*/
	 static const std::string normalizePath( const std::string &fpath ) ;
	/**
	* Removes the file protocol string at the beginning of the path if it is present
	* @param fpath the input pathname
        * @return the normalized pathname
	*/
	 static const std::string normalizeFile( const std::string &fpath ) ;
	 /**
         * removes white spaces form the begininng and from the end of the input string
         * @param str the input string
         * @return the string without white spaces
         */
         static const char* cleanString(char *str);
         /**
	* Converts the input integer to a string, adding a "zero"-digit ahead if it has one digit ( 0< d < 9)
	*/
	static std::string twoDigits(unsigned int d );
	/**
	*	Extract the filename from the input path (checking if the file exists on the local machine)
	*	@param path the input file pathname
	*	@return the name of the file
	*/
	static std::string  getFileName (const std::string& path) ;
	/**
	*	Compresses a file with gzip
	*	@param filename the pathname of the file to be compressed
	*	@param the path of the gzip file
	*/
	static std::string compressFile(const std::string &filename) ;
	/**
	*	Gets back the path of the URI removing information on the protocol, hostname and port
	*	@param uri the input URI
	*	@return the string with the absolute path extracted
	*/
 	static std::string getAbsolutePathFromURI (const std::string& uri) ;
	/**
	* Checks if a vector of strings contains a string item
	* @param vect the vector to be checked
	* @param item the string to be looked for
	*@return TRUE if the vector contains the string item, FALSE otherwise
	*/ 
	static bool contains (const std::vector<std::string> &vect, std::string item);
	 /**
        *	header for JobId output files
        */
        static const std::string JOBID_FILE_HEADER ;

private:
	/**
	* Check the WMS client installation path
	*/
	void checkPrefix();
	/**
	* Check the WMS client configuration file
	*/
	void checkConf();
	/** Look for possible configuration file */
	/**
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
	/**
        * Generates the log file.
	* The file is created only if either --debug or --logfile has been spevified among the user input options.
        * @return a pointer to a string with the pathname, NULL if no logfile has been generated
        */
        std::string* generateLogFile ( );
        /**
        * Gets the default Log File pathname
        * /<OutStorage-path>/<commandname>_<UID>_<PID>_<timestamp>.log
        */
        const std::string getDefaultLog ( );
        /**
	*	extract the tokens from the input string by a given separator
	*	<field_1><separator><fields_2>....... etc
	*	@param  instr input string
	*	@sep string separator between the tokens
	*	@return a vector of strings with the extracted tokens
	*/
	static const std::vector<std::string> extractFields(const std::string &instr, const std::string &sep);
        /**
        * perforns the base64 enconding of the input object
        *@param enc the object to be encoded
        *@param enc_size encoding size
        *@param out output string
        *@param out_max_size max size of the output string
        *@return the size of the output string, -1 in case of error during the encoding
        */
        static const int base64Encoder(void *enc, int enc_size, char *out,  int out_max_size);
	/**
        * converts the input string to a md5base64 string
        * @param the input string
        * @return the result of the conversion
        */
 	static  const char *str2md5Base64(const char *s);
        // Ad configuration files:
	glite::wms::common::configuration::WMCConfiguration *wmcConf;
	// Option files:
	Options *wmcOpts;
        // Ad utilities
        AdUtils *wmcAd;
        // debug flag (from options)
        bool debugInfo;
        // verbosity level
        LogLevel vbLevel ;
        //Log file pathname
        Log* logInfo ;
	// General configuration inner values
	std::string prefix;
}; // end class definition

} // glite
} // wms
} // client
} // utilities
#endif
