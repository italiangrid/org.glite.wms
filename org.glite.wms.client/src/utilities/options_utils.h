#include "glite/wms/jdl/Ad.h"

extern "C" {
#include <getopt.h>
}


class Options
{

	public:

		/*
		*	options attribute codes
		*/
		enum OptsAttributes{
			ALL = 500 ,
			CHKPT ,
			CONFIG ,
			CONFIGVO ,
			DBG , //debug
			DIR ,
			EXCLUDE ,
			FROM ,
			HELP ,
			INPUT ,
			LMRS ,
			LOGFILE ,
			NOGUI ,
			NOINT ,
			NOLISTEN ,
			NOMSG ,
			NOVERSION,
			OUTPUT ,
			PORT ,
			RANK ,
			RESOURCE ,
			STATUS ,
			TO ,
			USERTAG,
			VALID,
			VERBOSE ,
			VERSION ,
			VO
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
			JOBATTACH
		};
		/*
		*	default constructor
		*/
		Options (const WMPCommands &command);
		/*
		*	 reads the input options for the submission
		*	@param argv options
		*	@return the option object for the submission
		*/
		void readOptions(const int &argc, const char **argv);
		/*
		*	 check if the file exists
		*	@param file path
		*	@throw Exception if the file doesn't exist
		*/
		void checkFile (std::string file );
		/*
		*	gets the value of the option string-attribute
		*	@param attribute name of the attribute
		*/
		std::string* getStringAttribute (const Options::OptsAttributes &attribute);
		/*
		*	gets the value of the option string-attribute
		*	@param attribute name of the attribute
		*/
		long* getLongAttribute (const Options::OptsAttributes &attribute);
				/*
		*	gets the value of the option string-attribute
		*	@param attribute name of the attribute
		*/
		int* getIntAttribute (const Options::OptsAttributes &attribute);
		/*
		*	gets the value of the option string-attribute
		*	@param attribute name of the attribute
		*/
		bool getBoolAttribute (const Options::OptsAttributes &attribute);
		/*
		*	gets the value of the option list of strings-attribute
		*	@param attribute name of the attribute
		*/
		const std::vector<std::string> getListAttribute (const Options::OptsAttributes &attribute);
		/*
		*	gets the list of job identifiers
		*	@return a vector with the list of jobid's
		*/
		const std::vector<std::string> getJobIds( );
		/*
		*	gets the path to the JDL file
		*	@return the filepath string
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
		*	displays the usage help message for the output command
		*	@param exename name of the programme executable
		*	@param long displays the long usage help if it is "true"
		*/
		void output_usage(const char* &exename, const bool &long_usg=false) ;
		/*
		*	displays the usage help message for the attach command
		*	@param exename name of the programme executable
		*	@param long displays the long usage help if it is "true"
		*/
		void attach_usage(const char* &exename, const bool &long_usg=false) ;

	private:
		/*
		*	sets the value of the option attribute
		*	@param in_opt code assigned to the option
		*	(the last parameter in the long option struct assigned to each option)
		*	@param command line options
		*/
		void setAttribute (const int &in_opt, const char **argv);

		/*
		*	constants for help messages
		*/
		static const char* HELP_UI  ;
		static const char* HELP_VERSION  ;
		static const char* HELP_COPYRIGHT ;
		static const char* HELP_EMAIL ;
		/*
		*	long option std::strings
		*	(no short options defined for this set)
		*/
		static const char* LONG_ALL ;
		static const char* LONG_CHKPT	;
		static const char* LONG_CONFIGVO ;
		static const char* LONG_DEBUG ;
		static const char* LONG_DIR ;
		static const char* LONG_FROM ;
		static const char* LONG_HELP ;
		static const char* LONG_LMRS	;
		static const char* LONG_LOGFILE;
		static const char* LONG_NOGUI	;
		static const char* LONG_NOINT ;
		static const char* LONG_NOLISTEN ;
		static const char* LONG_NOMSG	;
		static const char* LONG_RANK ;
		static const char* LONG_TO	;
		static const char* LONG_USERTAG ;
		static const char* LONG_VERSION;
		static const char* LONG_VO	;


		/*
		*	long option std::strings and corresponding
		*	short option characters
		*/
		// output
		static const char* LONG_OUTPUT ;
		static const char SHORT_OUTPUT	 ;
		// input
		static const char* LONG_INPUT ;
		static const char SHORT_INPUT;
		// config
		static const char* LONG_CONFIG	;
		static const char SHORT_CONFIG	 ;
		// resource
		static const char* LONG_RESOURCE ;
		static const char SHORT_RESOURCE ;
		// valid
		static const char* LONG_VALID ;
		static const char SHORT_VALID ;
		// verbosity
		static const char* LONG_VERBOSE ;
		static const char SHORT_VERBOSE ;
		// status
		static const char* LONG_STATUS ;
		static const char SHORT_STATUS ;
		// exclude
		static const char* LONG_EXCLUDE ;
		static const char SHORT_EXCLUDE ;
		// port
		static const char* LONG_PORT ;
		static const char SHORT_PORT ;

		/*
		*	short usage constants
		*/
		static const std::string USG_ALL ;
		static const std::string USG_CHKPT	;
		static const std::string USG_CONFIG	;
		static const std::string USG_CONFIGVO ;
		static const std::string USG_DEBUG ;
		static const std::string USG_DIR ;
		static const std::string USG_EXCLUDE ;
		static const std::string USG_FROM ;
		static const std::string USG_HELP ;
		static const std::string USG_INPUT ;
		static const std::string USG_LMRS	;
		static const std::string USG_LOGFILE;
		static const std::string USG_NOGUI	;
		static const std::string USG_NOINT ;
		static const std::string USG_NOLISTEN ;
		static const std::string USG_NOMSG	;
		static const std::string USG_OUTPUT ;
		static const std::string USG_PORT ;
		static const std::string USG_RANK ;
		static const std::string USG_RESOURCE ;
		static const std::string USG_STATUS ;
		static const std::string USG_TO	;
		static const std::string USG_USERTAG ;
		static const std::string USG_VALID ;
		static const std::string USG_VERBOSE ;
		static const std::string USG_VERBOSITY ;
		static const std::string USG_VERSION;
		static const std::string USG_VO	;
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
		std::string* config ;
		std::string* configvo ;
		std::string* dir ;
		std::string* exclude ;
		std::string* from ;
		std::string* input ;
		std::string* lmrs ;
		std::string* logfile ;
		std::string* output ;
		std::string* resource ;
		std::string* status ;
		std::string* to ;
		std::string* vo ;
		/*
		*	boolean attributes
		*/
		bool all ;
		bool debug ;
		bool help ;
		bool nogui ;
		bool noint ;
		bool nolisten ;
		bool nomsg ;
		bool noversion ;
		bool rank ;
		bool version ;
		/*
		*	numerical attributes
		*/
		long *valid ;
		int *port ;
		int *verbosity ;
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
		/*
		*	pointer to the long options of the command
		*/
		const struct option *longOpts  ;
		/*
		*	pointer to the short options of the command
		*/
		char* shortOpts  ;
		/*
		*	type of user command
		*/
		WMPCommands cmdType ;
		/*
		*	path of the user JDL file
		*/
		std::string* jdlFile ;
		/*
		*	jobIds vector
		*/
		std::vector<std::string> jobIds ;

};
