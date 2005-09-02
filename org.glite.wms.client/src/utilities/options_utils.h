#ifndef GLITE_WMS_CLIENT_OPTIONSUTILS_H
#define GLITE_WMS_CLIENT_OPTIONSUTILS_H

#include <string>
#include <vector>

// WMProxy API's
#include "glite/wms/wmproxyapi/wmproxy_api.h"

#include "logman.h"

extern "C" {
#include <getopt.h>
}

namespace glite {
namespace wms{
namespace client {
namespace utilities {

std::string glite_wms_client_toLower ( const std::string &src);

class Options
{

	public:

		/*
		*	options attribute codes
		*/
		enum OptsAttributes{
                	NONE_ATTR = 1000 ,
			ALL ,
                        AUTODG ,
			CHKPT ,
                        COLLECTION,
			CONFIG ,
			DBG , //debug
                        DELEGATION,
			DIR ,
                        ENDPOINT,
			EXCLUDE ,
			FROM ,
			HELP ,
			INPUT ,
   			LISTONLY,
			LRMS ,
			LOGFILE ,
			NOGUI ,
			NOINT ,
			NOLISTEN ,
			NOMSG ,
			OUTPUT ,
			PORT ,
			PROTO,
			RANK ,
                        REGISTERONLY,
			RESOURCE ,
			START ,
			STATUS ,
			TO ,
			TRANSFER,
			USERTAG,
			VALID,
			VERBOSE ,
			VERSION,
                        VO
		} ;
                enum TimeOpts{
                	TIME_NO_OPT,
			TIME_FROM,
                        TIME_TO,
                        TIME_VALID
                } ;
		/*
		*	Type of command
		*/
		enum WMPCommands{
			JOBSUBMIT,
			JOBCANCEL,
			JOBSTATUS,
			JOBLOGINFO,
			JOBMATCH,
			JOBOUTPUT,
			JOBATTACH,
                        JOBDELEGATION
		};
		/*
		*	Default constructor
		*/
		Options (const WMPCommands &command);
		/*
		*	Default destructor
		*/
		 ~Options( );
		/*
		*	 reads the input options for the submission
		*	@param argv options
		*	@return a string with the list of the specified options
		*/
		std::string readOptions(const int &argc, const char **argv);



		/*
		*	 check if the file exists
		*	@param file path
		*	@throw Exception if the file doesn't exist
		*/
		void checkFile (std::string file );
		/*
		*	gets the value of the option string-attribute
		*	@param attribute name of the attribute
		*	@return the pointer to the string attribute value (NULL if the attribute has not been set)
		*/
		std::string* getStringAttribute (const Options::OptsAttributes &attribute);
		/*
		*	gets the value of the option string-attribute
		*	@param attribute name of the attribute
		*	@return the pointer to the int attribute value (NULL if the attribute has not been set)
		*/
		int* getIntAttribute (const Options::OptsAttributes &attribute);
		/*
		*	gets the value of the option string-attribute
		*	@param attribute name of the attribute
		*	@return the value of the boolean attribute
		*/
		bool getBoolAttribute (const Options::OptsAttributes &attribute);
		/*
		*	gets the value of the option list of strings-attribute
		*	@param attribute name of the attribute
		*	@return the vector with list
		*/
		const std::vector<std::string> getListAttribute (const Options::OptsAttributes &attribute);
                /*
		*	gets the short help usage message for an option attribute
		*	@param attribute name of the attribute
		*	@return the description string of the attribute
		*/
		const std::string getAttributeUsage (const Options::OptsAttributes &attribute);
		/*
		*	gets the list of job identifiers
		*	@return a vector with the list of jobid's
		*/
		const std::vector<std::string> getJobIds( );
		/*
		*	gets the path to the JDL file
		*	@return a pointer to the string of the JDL pathname
		*/
		std::string* Options::getPath2Jdl ();
		/*
		*	displays the usage help message for the submission
		*	@param exename name of the programme executable
		*	@param long displays the long usage help if it is "true"
		*/
		void submit_usage(const char* &exename, const bool &long_usg=false) ;
		/*
		*	displays the usage help message for the submission command
		*	@param exename name of the programme executable
		*	@param long displays the long usage help if it is "true"
		*/
		void status_usage(const char* &exename, const bool &long_usg=false) ;
		/*
		*	displays the usage help message for the status command
		*	@param exename name of the programme executable
		*	@param long displays the long usage help if it is "true"
		*/
		void loginfo_usage(const char* &exename, const bool &long_usg=false) ;
		/*
		*	displays the usage help message for the cancel command
		*	@param exename name of the programme executable
		*	@param long displays the long usage help if it is "true"
		*/
		void cancel_usage(const char* &exename, const bool &long_usg=false) ;
		/*
		*	displays the usage help message for the listmatch command
		*	@param exename name of the programme executable
		*	@param long displays the long usage help if it is "true"
		*/
		void lsmatch_usage(const char* &exename, const bool &long_usg=false) ;
		/*
		*	Displays the usage help message for the output command
		*	@param exename name of the programme executable
		*	@param long displays the long usage help if it is "true"
		*/
		void output_usage(const char* &exename, const bool &long_usg=false) ;
		/*
		*	Displays the usage help message for the attach command
		*	@param exename name of the programme executable
		*	@param long displays the long usage help if it is "true"
		*/
		void attach_usage(const char* &exename, const bool &long_usg=false) ;
		/*
		*	Displays the usage help message for the delegate-proxy command
		*	@param exename name of the programme executable
		*	@param long displays the long usage help if it is "true"
		*/
		void delegation_usage(const char* &exename, const bool &long_usg=false) ;
                /*
                *	Prints on the std output the help usage message for the command
                *	which was being handled and exits from the execution
                */
                void printUsage(const char* exename ) ;
                /*
                *	Gets a string with the name of the application
                *	@return the string with the name
                */
		std::string Options::getApplicationName() ;
                /**
                *	Gets the level of verbosity for the log info
                *	@return the level
                */
                const int getVerbosityLevel ( );
                /*
		*	Constants for the help and version messages
		*/
		static const char* HELP_UI  ;
		static const char* HELP_VERSION  ;
		static const char* HELP_COPYRIGHT ;
		static const char* HELP_EMAIL ;
                /*
		*	Constants for the verbosity level
		*/
		static const unsigned int DEFAULT_VERBOSITY;
		static const unsigned int MAX_VERBOSITY;
		/*
		* Default protocol for file transferring operations
		*/
		static const std::string TRANSFER_FILES_DEF_PROTO;
		/*
		* Default protocol for file archives and file compression
		*/
		static const std::string DESTURI_ZIP_PROTO;
		/*
		* Default protocol for file transferring operations by CURL
		*/
		static const std::string TRANSFER_FILES_CURL_PROTO;
		/*
		* LIst of protocol allowed for file transferring operations
		*/
		static const char* TRANSFER_FILES_PROTOCOLS[ ];

	private:
        	/*
                * Gets the default name of the application that is being executed
                */
        	std::string getDefaultApplicationName() ;
		/**
		*	Checks if an option is defined for a specific command (submit, cancel, etc...)
		*	using this class. It checks both long and short option. The check is based on
		*	the fields defined for the long option arrays (submitLongOpts, etc....)
		*	@param the option string to be checked
		*	@return 1 if the option is defined, 0 if it isn't and -1 if the input string doesn't represent a valid option
		*/
		const int checkOpts(const std::string& opt) ;
		/*
		* Maps the common short option to the correspondent OptsAttributes enumeration code
		* @param opt the short option code
		* @return the OptsAttributes enumeration code
		*/
		const int checkCommonShortOpts (const int &opt);
		/*
		*	sets the value of the option attribute
		*	@param in_opt code assigned to the option
		*	(the last parameter in the long option struct assigned to each option)
		*	@param command line options
                *	@param msg appends the information message string on the option has been set to this string
		*/
		void setAttribute (const int &in_opt, const char **argv, std::string &msg);

                /*
		*	long option std::strings
		*	(no short options defined for this set)
		*/
		static const char* LONG_ALL ;
		static const char* LONG_CHKPT	;
                static const char* LONG_COLLECTION;
		static const char* LONG_DEBUG ;
		static const char* LONG_DIR ;
		static const char* LONG_FROM ;
		static const char* LONG_HELP ;
		static const char* LONG_LISTONLY;
		static const char* LONG_LRMS	;
		static const char* LONG_LOGFILE;
		static const char* LONG_NOGUI	;
		static const char* LONG_NOINT ;
		static const char* LONG_NOLISTEN ;
		static const char* LONG_NOMSG	;
		static const char* LONG_PROTO	;
		static const char* LONG_RANK ;
                static const char* LONG_REGISTERONLY ;
		static const char* LONG_START ;
		static const char* LONG_TO	;
		static const char* LONG_TRANSFER	;
		static const char* LONG_USERTAG ;
		static const char* LONG_VERSION;
                static const char* LONG_VO;

		/*
		*	long option std::strings and corresponding
		*	short option characters
		*/
		// automatic delegation
		static const char* LONG_AUTODG ;
		static const char SHORT_AUTODG ;
		// output
		static const char* LONG_OUTPUT ;
		static const char SHORT_OUTPUT ;
		// input
		static const char* LONG_INPUT ;
		static const char SHORT_INPUT;
		// config
		static const char* LONG_CONFIG	;
		static const char SHORT_CONFIG ;
		// resource
		static const char* LONG_RESOURCE ;
		static const char SHORT_RESOURCE ;
		// valid & verbosity
		static const char* LONG_VALID ;
		static const char* LONG_VERBOSE ;
		static const char SHORT_V ;
		// status
		static const char* LONG_STATUS ;
		static const char SHORT_STATUS ;
		// exclude
		static const char* LONG_EXCLUDE ;
		static const char* LONG_ENDPOINT ;
		static const char SHORT_E ;

		// port
		static const char* LONG_PORT ;
		static const char SHORT_PORT ;
		// port
		static const char* LONG_DELEGATION ;
		static const char SHORT_DELEGATION ;

		static int FLAG_ENDPOINT ;
		static int FLAG_EXCLUDE ;

		/*
		*	short usage constants
		*/
		static const std::string USG_ALL ;
                static const std::string USG_AUTODG ;
		static const std::string USG_CHKPT	;
                static const std::string USG_COLLECTION	;
		static const std::string USG_CONFIG	;
		static const std::string USG_DEBUG ;
		static const std::string USG_DIR ;
                static const std::string USG_DELEGATION ;
                static const std::string USG_ENDPOINT ;
		static const std::string USG_EXCLUDE ;
		static const std::string USG_FROM ;
		static const std::string USG_HELP ;
		static const std::string USG_INPUT ;
                static const std::string USG_LISTONLY;
		static const std::string USG_LRMS	;
		static const std::string USG_LOGFILE;
		static const std::string USG_NOGUI	;
		static const std::string USG_NOINT ;
		static const std::string USG_NOLISTEN ;
		static const std::string USG_NOMSG	;
		static const std::string USG_OUTPUT ;
		static const std::string USG_PORT ;
		static const std::string USG_PROTO ;
		static const std::string USG_RANK ;
                static const std::string USG_REGISTERONLY ;
		static const std::string USG_RESOURCE ;
		static const std::string USG_START ;
		static const std::string USG_STATUS ;
		static const std::string USG_TO	;
		static const std::string USG_TRANSFER	;
		static const std::string USG_USERTAG ;
		static const std::string USG_VALID ;
		static const std::string USG_VERBOSE ;
		static const std::string USG_VERBOSITY ;
		static const std::string USG_VERSION;
                static const std::string USG_VO;
		/*
		*	require/not-require char for short option
		*	(used by definition of the short options)
		*/
		static const char short_required_arg ;
		static const char short_no_arg ;
		/*
		*	std::string attributes
		*/
		std::string* chkpt ;
                std::string* collection ;
		std::string* config ;
                std::string* delegation ;
		std::string* dir ;
		std::string* endpoint;
		std::string* exclude ;
		std::string* fileprotocol ;
		std::string* from ;
		std::string* input ;
		std::string* lrms ;
		std::string* logfile ;
		std::string* output ;
		std::string* resource ;
		std::string* start ;
		std::string* status ;
		std::string* to ;
		std::string* valid ;
		std::string* vo ;

		/*
		*	boolean attributes
		*/
		bool all ;
                bool autodg ;
		bool debug ;
		bool help ;
                bool listonly ;
		bool nogui ;
		bool noint ;
		bool nolisten ;
		bool nomsg ;
		bool rank ;
                bool registeronly;
		bool transfer ;
		bool version ;
		/*
		*	numerical attributes
		*/
		unsigned int *port ;
		unsigned int *verbosity ;
		/*
		*	listing-attributes
		*/
		std::vector<std::string> usertag;
		/*
		*	long options for each command
		*/
		static const struct option submitLongOpts[]  ;
		static const struct option statusLongOpts[]  ;
		static const struct option loginfoLongOpts[]  ;
		static const struct option cancelLongOpts[]  ;
		static const struct option lsmatchLongOpts[]  ;
		static const struct option outputLongOpts[]  ;
		static const struct option attachLongOpts[]  ;
                static const struct option delegationLongOpts[]  ;
		/*
		*	pointer to the long options of the command
		*/
		const struct option *longOpts  ;
		/*
		*	pointer to the short options of the command
		*/
		char* shortOpts  ;

		unsigned int numOpts ;
		/*
		*	type of user command
		*/
		WMPCommands cmdType ;
                /*
                *
                */
		std::string applName ;
		/*
		*	path of the user JDL file
		*/
		std::string* jdlFile ;
		/*
		*	jobIds vector
		*/
		std::vector<std::string> jobIds ;
		/*
                *
                */
                 LogLevel verbosityLevel ;
};
} // glite
} // wms
} // client
} // utilities
#endif

