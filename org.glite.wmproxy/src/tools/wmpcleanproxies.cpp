


#include <vector>
#include<iostream>
#include "utilities/wmputils.h"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/exception.hpp" //managing boost errors
#include "boost/lexical_cast.hpp"

#include "utils.h"

#define LOG(sev,m1,m2) if (logFile) {logFile->print (sev,m1,m2);}
#define LOGFILE(sev,m1,m2) if (logFile) {logFile->print (sev,m1,m2,false);}
#define FILL std::setw(2) << std::setfill('0')

#include "proxy_cleaner.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace tools {

namespace fs = boost::filesystem ;
namespace wmputils = glite::wms::wmproxy::utilities;
using namespace std;


const string monthStr[]  = {"Jan", "Feb", "March", "Apr", "May", "June" ,"July", "Aug", "Sept", "Oct", "Nov", "Dec"};
const char *LOG_FILENAME = "glite-wms-wmproxy-purge-proxycache";


enum ExcCode {
	INPUT_ERROR,
	INVALID_PROXY,
	PROXY_ERROR,
	INVALID_PATH,
 };

 /**
 * ProxyCache Exception
 */
class ProxyCacheException : public std::exception{
public:

ProxyCacheException (const string& source, int line_number, const string& method, int code, const string& exception)
	: error_code(code), exception_name(exception){
	line = line_number;
	source_file    = source;
	method_name    = method;
	// stack= "";
	line = 0;
};
~ProxyCacheException() throw(){ }

const char* what() const throw(){
	ostringstream e ;
	// Exception name should be displayed only once
	if (stack_strings.size()==0){ e << exception_name;};
	if (line !=0){ e <<"(line: " << line << ")\n";};
	//Adding error msg
	if (error_message!="") e << ": " << string(what());
	return (e.str()).c_str();
};

const int getCode( ){ return error_code;};

private :
	/**  integer error code representing the cause of the error */
	int                   error_code;
	/**  string exception message representation*/
	std::string          error_message ;
	/**  line number where the exception was raised */
	int                   line;
	/** The name of the file where the exception was raised */
	std::string          source_file;
	/** the name of the exception */
	std::string          exception_name;
	/** the name of the method where the expceiton was raised */
	std::string          method_name ;
	/** a string representation of the stacktrace */
	std::string          stack;
	/** the actual internal stacktrace representation */
	std::vector< std::string> stack_strings ;
	/** the name of the ancestor exception */
	std::string          ancestor ;
};

const char* VERSION_NUMBERS            = "1.0.0";

// Short-option chars
const char SHORT_VERSION	= 'v' ;
const char SHORT_HELP		= 'h' ;
const char LOG				= 'l' ;
const char DEBUG				= 'd' ;

/**
* LONG OPTIONS ===========================
*/
const struct option ProxyCleaner::longOpts[] = {
	{	Utils::LONG_LOG,       		required_argument,		0,		LOG},
	{	Utils::LONG_DEBUG,       	no_argument,			0,		DEBUG},
	{	Utils::LONG_VERSION,       	no_argument,			0,		SHORT_VERSION},
	{	Utils::LONG_HELP,       		no_argument,			0,		SHORT_HELP},
	{0, 0, 0, 0}
};

/**
* Default constructor
*/
ProxyCleaner::ProxyCleaner () {
	usrOpts.proxycache = NULL;
	usrOpts.log = NULL;
	usrOpts.debug = false ;
	usrOpts.version = false  ;
	usrOpts.help = false ;
	/**
	* Short Options
	*/
	asprintf (&shortOpts,
		"%c%c%c%c" ,
		SHORT_VERSION,    Utils::short_no_arg,
		SHORT_HELP, 	Utils::short_no_arg);
};

/**
* Default destructor
*/
ProxyCleaner::~ProxyCleaner () {
	if (usrOpts.proxycache) { delete(usrOpts.proxycache);}
	if (usrOpts.log) { delete(usrOpts.log);}
};


int ProxyCleaner::listFiles ( const std::string &proxycache, std::vector<std::string>& v, std::string& err ) {
	int ne = 0;
	err = "";
	try {
		const fs::path p(proxycache, fs::native);
		if (fs::exists(p) == false){
			throw ProxyCacheException(__FILE__, __LINE__, "listFiles",
				INVALID_PATH, "No such directory: " + proxycache);
		}
		if (fs::is_directory( p )==false){
			throw ProxyCacheException(__FILE__,  __LINE__, "listFiles",
				INVALID_PATH, "The path exists, but it is not a directory: " + proxycache);
		}
		fs::directory_iterator end_iter;
		for ( fs::directory_iterator dir_itr( p ); dir_itr != end_iter; ++dir_itr ) {
			try {
				if ( !fs::is_directory( *dir_itr ) ) {
					v.push_back( dir_itr->native_file_string() );
				}
			} catch ( const std::exception & ex ) {
					ne++ ;
					err += dir_itr->native_file_string() + " (" + ex.what() + ")\n";
			}
		}
	} catch (fs::filesystem_error &ex) {
		throw ProxyCacheException(__FILE__, __LINE__, "lisFiles",
		INVALID_PATH, "invalid directory (" + proxycache + "): " + ex.what( ));
	}
	return ne ;
}

std::string ProxyCleaner::twoDigits(unsigned int d ){
        ostringstream dd ;
        if (d<10){ dd << "0" << d ;}
        else { dd <<  d ;}
        return dd.str();
};

std::string ProxyCleaner::getTimeStamp() {
	string ws = " ";
	ostringstream px;
	time_t now = time(NULL);
	// PREFIX MSG: timestamp
        struct tm *ns = localtime(&now);
        // PREFIX MSG: time stamp: day-month
        px << twoDigits(ns->tm_mday) << ws << monthStr[ns->tm_mon] << ws << (ns->tm_year+1900) <<"," << ws;
        // PREFIX MSG: time stamp: hh::mm:ss
 	px << twoDigits(ns->tm_hour) << ":" << twoDigits(ns->tm_min) << ":" << twoDigits(ns->tm_sec) << ws << ns->tm_zone;
	return px.str();
}

/**
* Creates a log file to the specified path
*@param path the location of the log file
*/
Log*  ProxyCleaner::createLogFile (const std::string &path) {
	if (usrOpts.debug) {
		logFile = new Log(path, WMSLOG_DEBUG);
	} else {
		logFile = new Log(path, WMSLOG_INFO);
	}
	return logFile;
};

/**
* Returns the pointer to the log file
*@param path the location of the log file
*/
Log* ProxyCleaner::createLogFile ( ) {
	ostringstream oss;
	string path = "";
	char *env = NULL;
	//time stamp
	time_t now = time(NULL);
        struct tm *ns = localtime(&now);
        oss << (ns->tm_year+1900) << FILL << ns->tm_mon ;
	oss << FILL << ns->tm_mday ;
	// location
	env = getenv (Utils::GLITE_LOG_ENV);
	if (env) {
		path = string(env) ;
	} else {
		path = string(Utils::LOG_DEFAULT_PATH) ;
	}
	path += "/" + string(LOG_FILENAME) + "-" + oss.str( ) + ".log";
	// log object
	if (usrOpts.debug) {
		logFile = new Log(path, WMSLOG_DEBUG);
	} else {
		logFile = new Log(path, WMSLOG_WARNING);
	}
	return logFile ;
};

time_t ProxyCleaner::ASN1_UTCTIME_get(const ASN1_UTCTIME *s) {
        struct tm tm;
        int offset;
        memset(&tm,'\0',sizeof tm);
#define g2(p) (((p)[0]-'0')*10+(p)[1]-'0')
        tm.tm_year=g2(s->data);
        if(tm.tm_year < 50)
                tm.tm_year+=100;
        tm.tm_mon=g2(s->data+2)-1;
        tm.tm_mday=g2(s->data+4);
        tm.tm_hour=g2(s->data+6);
        tm.tm_min=g2(s->data+8);
        tm.tm_sec=g2(s->data+10);
        if(s->data[12] == 'Z')
                offset=0;
        else
                {
                offset=g2(s->data+13)*60+g2(s->data+15);
                if(s->data[12] == '-')
                        offset= -offset;
                }
#undef g2

    return timegm(&tm)-offset*60;
}

const long ProxyCleaner::get_time_left(const string &file) {
	ostringstream oss;
	time_t timeleft = 0;
	X509 *x = NULL;
	BIO* in = BIO_new(BIO_s_file());
	if (in) {
		BIO_set_close(in, BIO_CLOSE);
		if (BIO_read_filename( in, file.c_str() ) > 0) {
			x = PEM_read_bio_X509(in, NULL, 0, NULL);
			if (x) {
				const ASN1_UTCTIME *s = X509_get_notAfter(x);
				if (s){
					timeleft = ( ASN1_UTCTIME_get  (s)  - time(NULL) ) / 60 ;
				} else{
					throw ProxyCacheException(__FILE__,__LINE__,
					"X509_get_notAfter", PROXY_ERROR,
					"unable to get proxy time left");
				}
			} else{
				throw ProxyCacheException(__FILE__,__LINE__,
				"PEM_read_bio_X509", -1,
				"it is not a proxy file");
			}

		} else {
			BIO_free(in);
			throw ProxyCacheException(__FILE__,__LINE__,
				"BIO_read_filename", -1,
				"Unable to get the proxy time left");
		}
		BIO_free(in);
		if (x) {delete(x);}
	}else{
			throw ProxyCacheException(__FILE__,__LINE__,
				"BIO_read_filename", -1,
			"Unable to get the proxy time left (BIO ssl error)");
	}
	oss << "TimeLeft : " << timeleft << " sec";
	LOG (WMS_DEBUG, oss.str(), "");
	return timeleft;
}

/**
* Cancels the expired proxies in the proxycache directory
*/
std::string ProxyCleaner::removeExpiredProxies(string& proxycache){
	string err="";
	string msg="";
	vector<string> proxies;
	vector<string>::iterator it ;
	int nd = 0;
	int valid = 0;
	int ne = 0;
	ne = listFiles(proxycache, proxies, err);
	if (proxies.empty()) {
		msg = "The directory is empty: " + proxycache;
		LOG (WMS_DEBUG, msg, "");
	} else {
		msg = "Number of files found in the directory : " +
			boost::lexical_cast<std::string>(proxies.size()) ;
		LOG (WMS_DEBUG, msg, "");
		msg += "\n";
		for ( it = proxies.begin( ) ; it != proxies.end( ) ; ++it ) {
			try {
				LOG (WMS_DEBUG, "Processing file:" , (*it));
				if ( get_time_left(string(*it)) <= 0){
					LOG (WMS_DEBUG, "Expired proxy:" , "removing the file ...");
					if ( remove(it->c_str()) != 0) {
						ne++;
						err += "\t"+ (*it) + " (unable to remove the file)";
					} else  {nd++;}
				} else {
					LOG (WMS_DEBUG, "The proxy is still valid", "");
					valid ++ ;
				}
			} catch (ProxyCacheException &exc) {
				ne++;
				string e = exc.what( ) ;
				if (e.size() == 0) { e = "it is not proxy file";  }
				err += "  " + (*it) + " (" + e + ")\n";
			}
		}
		if (nd>0) {
			msg += "- " + boost::lexical_cast<std::string>(nd) + " expired proxy ";
			if (nd==1){
				msg+=  "file has been removed.\n";
			} else{
				msg+=  "files have been removed.\n";
			}
		} else {
			msg+=  "- no expired proxy files have been found.\n";
		}
		if ( valid == 1 ) {
			msg += "- 1 file is still valid.\n";
		} else if ( valid > 1 ) {
			msg += "- " + boost::lexical_cast<std::string>(valid) + " files are still valid.\n";
		}
		if (ne==1){
			msg += "- couldn't remove the following file :\n" + err ;
		} else if (ne>1){
			msg += "- couldn't remove " + boost::lexical_cast<std::string>(ne) + " files :\n" + err ;
		}
}
	return msg ;
}
std::string ProxyCleaner::getResultMsg (std::string &msg) {
	ostringstream out ;
	out << "=======================================================================================================\n\n";
	out << "\t\tProxyCache Purger results:\n\n";
	out << msg ;
	out << "\n=======================================================================================================\n";

	if (this->logFile) {
		string *log = logFile->getPathName( );
		if (log) {
			out << "\nPossible Errors and Debug messages have been printed in the following file:\n";
       			out << *log << "\n";
		}
	}
	out << "\n";
	LOGFILE (WMS_INFO, msg, "");
	return out.str( );
}
const int ProxyCleaner::checkOpts(const std::string& opt) {
	int r = Utils::ERROR;
	unsigned int numOpts = (sizeof(ProxyCleaner::longOpts)/sizeof(option)) -1;
	if (opt.compare (0,2,"--")==0){
		string lg = opt.substr(2, (opt.size()-2));
		for (unsigned int i = 0; i < numOpts ; i++){
			struct option s = ProxyCleaner::longOpts[i];
			if (lg.compare(s.name)==0){
				r = Utils::SUCCESS ;
				break;
			}
		}
	} else if (opt.compare (0,1,"-")==0){
		for (unsigned int i = 0; i < numOpts ; i++){
			struct option s = ProxyCleaner::longOpts[i];
			string sh = opt.substr(1, (opt.size()-1));
			if (sh.size()==1){
				if (s.val < 128){
					char c = (char)s.val;
					if (sh.compare(string(1,c))==0){
						r = Utils::SUCCESS ;
						break;
					}
				}
			}
		}
	}
	return r ;
};
const int ProxyCleaner::readOptions(int argc,char **argv, cleanerOpts &opts, string &msg, string &errors){
	int result = Utils::SUCCESS;
	string arg = "";
	int next_opt = 0;
	char *env = NULL;
	string err = "";
	msg = "Command: " + string(argv[0]) ;
	// Reading options ......
	if (argc>1){
		// option parsing
		while (next_opt != -1){
			// string with the option currently being parsed
			if (optind < argc){ arg = argv[optind]; }
			else { arg = ""; }
			// Returns the "val"-field of the struct "option"
			next_opt = getopt_long (argc, (char* const*)argv,
						shortOpts, ProxyCleaner::longOpts, NULL);
			// error
			if (next_opt == '?') {
				errors = "\nError: Invalid Option";
				result = Utils::ERROR;
				break;
			} else if ( next_opt != -1 && arg.size() > 0 && checkOpts(arg) != Utils::SUCCESS  ){
				errors  ="\nError: --" + arg + ": unrecognized option";
				result = Utils::ERROR;
				break;
			} else if (next_opt != -1 ){
				switch (next_opt) {
					case (LOG) : {
						opts.log = new string(utilities::getAbsolutePath(optarg));
						msg += " --" + string(Utils::LONG_LOG) + " " + *opts.log;
						break;
					}
					case (DEBUG) : {
						opts.debug = true;
						msg += " --" + string(Utils::LONG_DEBUG) ;
						break;
					}
					case (SHORT_VERSION) : {
						opts.version = true;
						msg += " --" + string(Utils::LONG_VERSION) ;
						break;
					}
					case (SHORT_HELP) : {
						opts.help = true;
						msg += " --" + string(Utils::LONG_HELP) ;
						break;
					}
					default: {
						// do nothing
						break;
					}
				}
			}
		}
		if (opts.version) {
			cout << version( ) << "\n";
			exit(0);
		} else if (opts.help) {
			usage(argv[0]);
		}
		if (optind == argc){
			throw ProxyCacheException (__FILE__,__LINE__,
				"readOptions", INPUT_ERROR,
				"The last argument must be a path to the proxycache directory to be purged." );
		}
		else if (optind < (argc-1) ){
			throw ProxyCacheException (__FILE__,__LINE__,
				"readOptions",INPUT_ERROR,
				"Wrong Input Argument: " + string(argv[optind]) );
		} else if ( optind == (argc-1) ) {
			usrOpts.proxycache = new string ( );
			*usrOpts.proxycache = wmputils::getAbsolutePath(argv[optind]);
			*usrOpts.proxycache = wmputils::normalizePath(*usrOpts.proxycache);
		}
	} else {
		usage(argv[0]);
	}
	return result;
} ;
/**
* Returns the pointer to the userOptions struct
*/
cleanerOpts* ProxyCleaner::getUserOptions( ) {
	return (&usrOpts);
};

const std::string ProxyCleaner::version() {
	string m = "Proxy Purger version ";
	m += string(VERSION_NUMBERS) + "\n";
	m +=	"Copyright (C) 2006 by DATAMAT SpA";
	return m;
};
/**
* Help usage message
*/
void ProxyCleaner::usage(char *exe){
	cout << "Usage : " << exe << "[options] <directory path> \n";
	cout << "where\n";
	cout << "\t[options]:\n";
	cout << "\t\t--" << Utils::LONG_LOG << " <filepath>\n";
	cout << "\t\t\tcreate a log file in a different path from the default one\n";
	cout << "\t\t--" << Utils::LONG_DEBUG << "\n";
	cout << "\t\t\tprint debug messages on the std output;\n";
	cout << "\t\t--" << Utils::LONG_VERSION << ", -v\n";
	cout << "\t\t\tprint the version message on the std output;\n";
	cout << "\t\t--" << Utils::LONG_HELP << ", -?\n";
	cout << "\t\t\tprint this help message on the std output;\n\n";
	cout << "\t<directory path>=pathname to the WMS proxy cache location (mandatory)\n\n";
	cout << "\nPlease report any bug at:\n";
	cout << "\tegee@datamat.it\n\n";
	exit (0);
}

void ProxyCleaner::error (const std::string &msg, const std::string &details) {
	LOG (WMS_ERROR, msg, details);
	exit(-1);
};
}}}} // ending namespaces

using namespace glite::wms::wmproxy::tools ;

int main (int argc,char **argv){
	ProxyCleaner cleaner ;
	string msg = "";
	int result = 0;
	string err = "";
	int exit_code = 0;
	string proxycache = "";
	Log *logFile = NULL;
	try{
		// reads the input options
		cleanerOpts *opts = cleaner.getUserOptions( );
		result = cleaner.readOptions (argc, argv, *opts, msg, err);
		if (opts->log){
			logFile = cleaner.createLogFile(*(opts->log));
		} else {
			logFile = cleaner.createLogFile( );
		}
		LOG (WMS_DEBUG, msg, "");
		if (result == Utils::ERROR){
			cleaner.error(err, "");
		}
		if (opts->proxycache==NULL){
			throw ProxyCacheException (__FILE__,__LINE__,
				"main", INPUT_ERROR,
				"The last argument must be a path to the proxycache directory to be purged.");
		}
		LOG (WMS_INFO, "Purging expired proxies in the cache directory:", *opts->proxycache);
		string msg = cleaner.removeExpiredProxies (*opts->proxycache);
		LOG (WMS_DEBUG, "Ending of proxy purging", "");
		cout << cleaner.getResultMsg(msg);
	} catch (ProxyCacheException &exc){
		if (exc.getCode()==INVALID_PATH ){
			cerr << "Error -  invalid proxy cache path (" << exc.what() << ")\n";
		} else{
			cerr << "Error: " << exc.what( ) << "\n";
		}
	}
 	return exit_code;
}
