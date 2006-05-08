
#include <vector>
#include<iostream>
#include "utilities/wmputils.h"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/exception.hpp" //managing boost errors
#include "boost/lexical_cast.hpp"
#include <openssl/pem.h>

namespace fs = boost::filesystem ;
namespace utils = glite::wms::wmproxy::utilities;
using namespace std;

 const string monthStr[]  = {"Jan", "Feb", "March", "Apr", "May", "June" ,"July", "Aug", "Sept", "Oct", "Nov", "Dec"};

enum ExcCode{
	INVALID_PROXY,
	PROXY_ERROR,
	INVALID_PATH,
 };

 /**
 * ProxyCache Exception
 */
class ProxyCacheException : public std::exception{
public:

 ProxyCacheException (const string& source, const string& method, int code, const string& exception)
	: error_code(code), exception_name(exception){
	source_file    = source;
	method_name    = method;
	// stack= "";
	line = 0;
};
~ProxyCacheException() throw(){ }

const char* what() const throw(){
   string result ;
   result = "";
   // Exception name should be displayed only once
   if (stack_strings.size()==0){result +=exception_name;};
   //Adding error msg
   if (error_message!="") result +=": " + string(what());
   return result.c_str();
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

int listFiles ( const fs::path& p, vector<string>& v, string& err ) {
	int ne = 0;
	err = "";
	if ( fs::exists(p) && fs::is_directory( p ) )  {
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
	}
	return ne ;
}

std::string twoDigits(unsigned int d ){
        ostringstream dd ;
        if (d<10){ dd << "0" << d ;}
        else { dd <<  d ;}
        return dd.str();
};

std::string getTimeStamp() {
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

time_t ASN1_UTCTIME_get(const ASN1_UTCTIME *s) {
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

const long get_time_left(const string &file) {
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
					throw ProxyCacheException(__FILE__,
					"X509_get_notAfter", PROXY_ERROR,
					"unable to get proxy time left");
				}
			} else{
				throw ProxyCacheException(__FILE__,
				"PEM_read_bio_X509", -1,
				"invalid proxy file");
			}

		} else {
			BIO_free(in);
			throw ProxyCacheException(__FILE__,
				"BIO_read_filename", -1,
				"Unable to get the proxy time left");
		}
		BIO_free(in);
		if (x) {delete(x);}
	}else{
			throw ProxyCacheException(__FILE__,
				"BIO_read_filename", -1,
			"Unable to get the proxy time left (BIO ssl error)");
	}
	return timeleft;
}

/**
* Cancels the expired proxies in the proxycache directory
*/
std::string removeExpiredProxies(string& proxycache){
	string err="";
	string msg="";
	vector<string> proxies;
	vector<string>::iterator it ;
	int nd = 0;
	int valid = 0;
	int ne = 0;
	try{
		const boost::filesystem::path cp(proxycache, boost::filesystem::native);
		ne = listFiles(cp, proxies, err);
		if ( !  fs::is_directory(cp) ) {
			throw ProxyCacheException(__FILE__, "removeExpiredProxies",
				INVALID_PATH, "No such directory: " + proxycache);
		} else {	
			proxycache = utils::getAbsolutePath(proxycache);
			proxycache = utils::normalizePath(proxycache);
			 cout << "\nStarted on " << getTimeStamp() << "\n";
			cout << "\nPurging the proxy cache at : " << proxycache << "\n\n";
		}
	}catch (fs::filesystem_error &ex){
		throw ProxyCacheException(__FILE__, "removeExpiredProxies",
		INVALID_PATH, "invalid directory (" + proxycache + "): " + ex.what( ));
	}
		if (proxies.empty()) {
			msg = "The directory is empty: " + proxycache;
		} else {
			msg = "Number of files found in the directory : " + boost::lexical_cast<std::string>(proxies.size()) + "\n";
			for ( it = proxies.begin( ) ; it != proxies.end( ) ; ++it ) {
				try {
					if ( get_time_left(string(*it)) <= 0){
						if ( remove(it->c_str()) != 0) {
							ne++;
							err += "\t- "+ (*it) + " (unable to remove the file)";
						} else  {nd++;}
					} else {
						valid ++ ;
					}
				} catch (ProxyCacheException &exc) {
					ne++;
					string e = exc.what( ) ;
					if (e.size() == 0) { e = "invalid proxy file";  } 
					err += " - " + (*it) + " (" + e + ")\n";
				}
			}

			if (nd>0) {
				msg += "> " + boost::lexical_cast<std::string>(nd) + " expired proxy ";
				if (nd==1){
					msg+=  "file has been removed.\n";
				} else{
					msg+=  "files have been removed.\n";
				}
			} else {
				msg+=  "> No expired proxy files have been found.\n";
			}
			if ( valid == 1 ) {
				msg += "> 1 file is still valid.\n";
			} else if ( valid > 1 ) {
				msg += "> " + boost::lexical_cast<std::string>(valid) + " files are still valid.\n";
			}
			if (ne==1){
				msg += "> couldn't remove the following file :\n" + err ;
			} else if (ne>1){
				msg += "> couldn't remove " + boost::lexical_cast<std::string>(ne) + " files :\n" + err ;
			}
	}
	return msg ;
}
/**
* Help usage message
*/
void usage(char *exe){
	cerr << "Usage : " << exe << " <directory path>\n";
	cerr << "where: <directory path> = pathname to WMS proxy cache location\n";
	exit (0);
}

int main (int argc,char **argv){
	int exit_code = 0;
	string path = "";
	if ( argc < 2 || strcmp(argv[1],"--help") == 0 ) {
		usage(argv[0]);
	} else if (argc > 2) {
		cerr << "Error - " << argv[2] << ": invalid argument\n";
		usage(argv[0]);
		exit_code = -1;
	} else {
		try{
			path = utils::normalizePath(argv[1]);
			string msg = removeExpiredProxies (path);
			cout << "================================================================\n\n";
			cout << "\tPROXY-CACHE PURGER\n\n";
			cout << msg << "\n";
			cout << "================================================================\n";

		} catch (ProxyCacheException &exc){
			if (exc.getCode()==INVALID_PATH ){
				cerr << "Error -  invalid proxy cache path (" << exc.what() << ")\n";
			} else{
				cerr << "Error: " << exc.what( ) << "\n";
			}
		}
	}
 	return exit_code;
}
