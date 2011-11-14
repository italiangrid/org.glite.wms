/*
Copyright (c) Members of the EGEE Collaboration. 2004. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

    http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
See the License for the specific language governing permissions and 
limitations under the License.
*/

//
// Author: Marco Sottilaro <marco.sottilaro@datamat.it>
//

#include "converter.h"

#include "stdio.h"
#include <iostream>
#include <fstream>
#include <sstream>
// time fncts
#include "time.h"
#include <iomanip>

#include "utilities/wmputils.h"
#include "security/wmpauthorizer.h"

#include "utils.h"

#define LOG(sev,m1,m2) if (logFile) {logFile->print (sev,m1,m2);}
#define LOGFILE(sev,m1,m2) if (logFile) {logFile->print (sev,m1,m2,false);}
#define FILL std::setw(2) << std::setfill('0')


using namespace std;
using namespace glite::wms::wmproxy::authorizer;
namespace utilities = glite::wms::wmproxy::utilities;

namespace glite {
namespace wms {
namespace wmproxy {
namespace tools {

const char* VERSION_NUMBERS            = "1.0.0";
const char *LOG_FILENAME = "glite-wms-wmproxy-gridmapfile2gacl";
/**
* LONG OPTIONS ===========================
*/

// Short-option chars
const char SHORT_INPUT		= 'i' ;
const char SHORT_OUTPUT		= 'o' ;
const char SHORT_VERSION	= 'v' ;
const char SHORT_HELP		= 'h' ;
const char NOINT				= 't' ;
const char LOG				= 'l' ;
const char DEBUG				= 'd' ;
const char NEW				= 'n' ;
const char ADD				= 'a' ;
const char REMOVE			= 'r' ;
const char DN					= 'p' ;
const char FQAN				= 'f' ;

/**
* LONG OPTIONS ===========================
*/
const struct option Converter::longOpts[] = {
	{	Utils::LONG_INPUT,           	required_argument,		0,		SHORT_INPUT},
	{	Utils::LONG_OUTPUT,        	required_argument,		0,		SHORT_OUTPUT},
	{	Utils::LONG_LOG,       		required_argument,		0,		LOG},
	{	Utils::LONG_NEW,       		no_argument,			0,		NEW},
	{	Utils::LONG_DEBUG,       	no_argument,			0,		DEBUG},
	{	Utils::LONG_NOINT,       		no_argument,			0,		NOINT},
	{	Utils::LONG_ADD,       		no_argument,			0,		ADD},
	{	Utils::LONG_REMOVE,       	no_argument,			0,		REMOVE},
	{	Utils::LONG_DN,       		no_argument,			0,		DN},
	{	Utils::LONG_FQAN,       		no_argument,			0,		FQAN},
	{	Utils::LONG_VERSION,       	no_argument,			0,		SHORT_VERSION},
	{	Utils::LONG_HELP,       		no_argument,			0,		SHORT_HELP},
	{0, 0, 0, 0}
};

/**
* ===== class Converter =======================================
*/

/**
* Default constructor
*/
Converter::Converter( ) {
	// UserOption struct
	usrOpts.input = NULL;
	usrOpts.output = NULL;
	usrOpts.log = NULL;
	usrOpts.debug = false;
	usrOpts.noint= false;
	usrOpts.create = false;
	usrOpts.add = false;
	usrOpts.remove = false;
	usrOpts.dn = false;
	usrOpts.fqan = false;
	usrOpts.version = false;
	usrOpts.help = false;
	// logFile
	logFile = NULL;
	// path to gridmap file
	gridMapFile = "";
	// new gacl has been created
	newGacl = false;
	existingGacl = false;
	// operation counters
	readEntries = 0;
	addedEntries = 0;
	removedEntries = 0;
	notReplacedEntries = 0;
	// operations
	addOper = false;
	rmOper = false;
	/**
	* Short Options
	*/
	asprintf (&shortOpts,
			"%c%c%c%c%c%c%c%c" ,
			SHORT_INPUT,		Utils::short_required_arg,
			SHORT_OUTPUT,		Utils::short_required_arg,
			SHORT_VERSION,    	Utils::short_no_arg,
			SHORT_HELP, 		Utils::short_no_arg
	);
};
/**
* Default destructor
*/
Converter::~Converter() {
	if (usrOpts.input) { delete(usrOpts.input ); }
	if (usrOpts.output) { delete(usrOpts.output); }
	if (usrOpts.log) { delete(usrOpts.log); }
	if (logFile) { delete(logFile); }

};

const std::string Converter::version() {
	string m = "GridMapFile2Gacl Converter version ";
	m += string(VERSION_NUMBERS) + "\n";
	m +=	"Copyright (C) 2006 by DATAMAT SpA";
	return m;
};

void Converter::usage(const std::string &exe) {
	cout << version() << "\n\n";
	cout << "usage:  glite-wms-wmproxy-gridmapfile2gacl [options]\n";
	cout << "where [options]:\n";
	cout << "\t--" << Utils::LONG_INPUT << ", -i <filepath>\n";
	cout << "\t\tload a gridmap file different from the default one\n";
	cout <<"\t\t(/etc/grid-security/grid-mapfile);\n";
	cout << "\t--" << Utils::LONG_OUTPUT << ", -o <filepath>\n";
	cout << "\t\tcreate a gacl file different from the default one\n";
	cout << "\t\t($GLITE_LOCATION/etc/glite_wms_wmproxy.gacl);\n";
	cout << "\t--" << Utils::LONG_LOG << " <filepath>\n";
	cout << "\t\tcreate a log file in a different path from the default one\n";
	cout << "\t\t([$GLITE_LOCATION_LOG] | [/tmp] /"<< LOG_FILENAME << "-<date>-.log);\n";
	cout << "\t--" << Utils::LONG_ADD << "\n";
	cout << "\t\tonly adding of new entries to the gacl file is allowed;\n";
	cout << "\t--" << Utils::LONG_REMOVE << "\n";
	cout << "\t\tonly removal of gacl entries (not present in the grid-mapfile) is allowed;\n";
	cout << "\t--" << Utils::LONG_NEW << "\n";
	cout << "\t\toverwrite the gacl file if it already exists;\n";
	cout << "\t--" << Utils::LONG_DN << "\n";
	cout << "\t\tupdate only dn-entries (person-entries);\n";
	cout << "\t--" << Utils::LONG_FQAN << "\n";
	cout << "\t\tupdate only fqan-entries (voms-entries);\n";
	cout << "\t--" << Utils::LONG_DEBUG << "\n";
	cout << "\t\tprint debug messages on the std output;\n";
	cout << "\t--" << Utils::LONG_VERSION << ", -v\n";
	cout << "\t\tprint the version message on the std output;\n";
	cout << "\t--" << Utils::LONG_HELP << ", -?\n";
	cout << "\t\tprint this help message on the std output;\n";
	cout << "\n\tPlease report any bug at:\n";
	cout << "\t\tegee@datamat.it\n\n";
};

void Converter::error (const std::string &msg, const std::string &details) {
	LOG (WMS_ERROR, msg, details);
	exit(-1);
};
/**
* Creates a log file to the specified path
*@param path the location of the log file
*/
Log* Converter::createLogFile (const std::string &path) {
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
Log* Converter::createLogFile ( ) {
	ostringstream oss;
	string path = "";
	char *env = NULL;
	//time stamp
	time_t now = time(NULL);
        struct tm *ns = localtime(&now);
        oss << (ns->tm_year+1900) << FILL << (ns->tm_mon+1) ;
	oss << FILL << ns->tm_mday ;
	// location
	env = getenv (Utils::GLITE_LOG_ENV);
	if (env) {
		path = string(env) ;
	} else {
		path = string(Utils::LOG_DEFAULT_PATH) ;
	}
	path +=  "/" + string(LOG_FILENAME) + "-" + oss.str( ) + ".log";
	// log object
	if (usrOpts.debug) {
		logFile = new Log(path, WMSLOG_DEBUG);
	} else {
		logFile = new Log(path, WMSLOG_WARNING);
	}
	return logFile ;
};
/**
* Returns the pointer to the userOptions struct
*/
converterOpts* Converter::getUserOptions( ) {
	return (&usrOpts);
};
/**
* Check whether an option is correct
*/
const int Converter::checkOpts(const std::string& opt) {
	int r = Utils::ERROR;
	unsigned int numOpts = (sizeof(Converter::longOpts)/sizeof(option)) -1;
	if (opt.compare (0,2,"--")==0){
		string lg = opt.substr(2, (opt.size()-2));
		for (unsigned int i = 0; i < numOpts ; i++){
			struct option s = Converter::longOpts[i];
			if (lg.compare(s.name)==0){
				r = Utils::SUCCESS ;
				break;
			}
		}
	} else if (opt.compare (0,1,"-")==0){
		for (unsigned int i = 0; i < numOpts ; i++){
			struct option s = Converter::longOpts[i];
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
/**
* Reads the input options
*/
const int Converter::readOptions(int argc,char **argv, converterOpts &opts, string &msg, string &errors){
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
					shortOpts, Converter::longOpts, NULL);
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
					case (SHORT_INPUT) : {
						opts.input = new string(utilities::getAbsolutePath(optarg));
						msg += " --" + string(Utils::LONG_INPUT) + " " + *opts.input ;
						break;
					}
					case (SHORT_OUTPUT) : {
						opts.output = new string(utilities::getAbsolutePath(optarg));
						msg += " --" + string(Utils::LONG_OUTPUT) + " " + *opts.output ;
						break;
					}
					case (LOG) : {
						opts.log = new string(utilities::getAbsolutePath(optarg));
						msg += " --" + string(Utils::LONG_LOG) + " " + *opts.log;
						break;
					}
					case (NOINT) : {
						opts.noint = true;
						msg += " --" + string(Utils::LONG_NOINT) ;
						break;
					}
					case (DEBUG) : {
						opts.debug = true;
						msg += " --" + string(Utils::LONG_DEBUG) ;
						break;
					}
					case (NEW) : {
						opts.create = true;
						msg += " --" + string(Utils::LONG_NEW) ;
						break;
					}
					case (ADD) : {
						opts.add = true;
						msg += " --" + string(Utils::LONG_ADD) ;
						break;
					}
					case (REMOVE) : {
						opts.remove = true;
						msg += " --" + string(Utils::LONG_REMOVE) ;
						break;
					}
					case (DN) : {
						opts.dn = true;
						msg += " --" + string(Utils::LONG_DN) ;
						break;
					}
					case (FQAN) : {
						opts.fqan = true;
						msg += " --" + string(Utils::LONG_FQAN) ;
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
		} ;
		// pathname to grid-mapfile
		if (opts.input==NULL){
			env = getenv (Utils::GRIDMAP_ENV);
			if (env) {
				opts.input = new string(env);
			} else {
				opts.input = new string(Utils::DEFAULT_GRIDMAPFILE);
			}
		}
		if (opts.output==NULL) {
			env = getenv (Utils::GLITE_LOCATION_ENV);
			if (env) {
				opts.output = new string(utilities::normalizePath(string(env)) + "/" +string(Utils::GACL_RELATIVE_PATH)  );
			} else {
				opts.output = new string();
				*opts.output = string(Utils::DEFAULT_GLITE_LOCATION) + string(Utils::GACL_RELATIVE_PATH);
			}
		}
		// --only-dn and --only-fqan
		 if (opts.dn && opts.fqan) {
		 	if (opts.dn) { err =  "--" + string(Utils::LONG_DN) + " - ";}
			if (opts.fqan) { err =  "--" + string(Utils::LONG_FQAN) + " - ";}
			err += "these options cannot be used together:\n";
			err += "--" + string(Utils::LONG_DN) + " | ";
			err += "--" + string(Utils::LONG_FQAN) + ".\n";
			error( "incompatible options\n", err);
		}
		// --only-add and --only-remove
		 if (opts.add && opts.remove) {
		 	if (opts.add) { err =  "--" + string(Utils::LONG_ADD) + " - ";}
			if (opts.remove) { err =  "--" + string(Utils::LONG_REMOVE) + " - ";}
			err += "these options cannot be used together:\n";
			err += "--" + string(Utils::LONG_ADD) + " | ";
			err += "--" + string(Utils::LONG_REMOVE) + ".\n";
			error( "incompatible options\n", err);
		} else {
			if (opts.add) {
				addOper = true;
			} else if (opts.remove)  {
				rmOper = true;
			} else {
				addOper = true;
				rmOper = true;
			}
		}
	}
	if (opts.input==NULL && opts.output==NULL) {
		// Default Grid-mapfile
		env = getenv (Utils::GRIDMAP_ENV);
		if (env) {
			opts.input = new string(env);
		} else {
			opts.input = new string(Utils::DEFAULT_GRIDMAPFILE);
		}
		// Default gacl file
		env = getenv (Utils::GLITE_LOCATION_ENV);
		if (env) {
			opts.output = new string(utilities::normalizePath(string(env)) + "/" +string(Utils::GACL_RELATIVE_PATH)  );
		} else {
			opts.output = new string();
			*opts.output = string(Utils::DEFAULT_GLITE_LOCATION) + string(Utils::GACL_RELATIVE_PATH);
		}
		cout << "Conversion of the grid-mapfile entries to gacl entries:\n";
		cout << "Working on files:\n";
		cout << "- grid-mapfile: " <<  *opts.input   << "\n";
		cout << "- gacl-file: " <<  *opts.output   << "\n\n";
		if (answerYes("Do you wish to continue", false, true)==false){
			cout << "bye\n";
			exit(0);
		}
	}
	return result;
} ;

/**
* Extracts the <dn> or <fqan> from the input entry 
*/
int Converter::getEntry(std::string &entry) {
	int result = Utils::SUCCESS;
	unsigned int p = 0;
	unsigned int len = 0;
	string s1 = "";
	string s2 = "";
	// format are  "<dn>" user ; "<fqan>" user
	if (entry.substr(0,1) == "\"") {
		entry = entry.substr(1,(entry.size()-1));
		p = entry.find("\"");
		if (p != string::npos ) {
			entry = entry.substr(0, p);
			entry = utilities::cleanString(entry);
			// checks for jolly chars (=all values)
			while (true) {
				p = entry.find (Utils::ALL_VALUES);
				if (p != string::npos ){
					s1 = entry.substr(0, p);
					p += Utils::ALL_VALUES.size();
					len = entry.size() ;
					if (len > p ){
						s2 = entry.substr(p,len);
						if (s2.compare(0,1,"/")!=0 && s1.compare(s1.size(),1,"/")!=0) {
							entry = s1 + "/" + s2;
						} else {
							entry = s1 + s2;
						}
					} else {
						entry = s1;
					}
				} else {
					break;
				}
			}

		} else {
			result = Utils::ERROR;
		}
	} else {
		result = Utils::ERROR;
	}

	return result;
}
/*
*: Loads the grid-mapfile located in "path" and
* puts the entries in the mapEntries vector;
* returns the number of entries in the grid map file; or ERROR
*
*/
int Converter::readMapFile(const string &path, std::vector<std::string> &entries, std::string &errors) {
	ostringstream os1;
	ostringstream os2;
  	string s = "";
	string e = "";
	int n = 0;
	int result = Utils::SUCCESS ;
	LOG (WMS_INFO, "Reading the grid-mapfile", path);
	this->gridMapFile = path;
  	ifstream in((const char*)path.c_str());
	if (in.is_open()) {
 		while(getline(in, s)) {
			s = utilities::cleanString(s);
			if (s.size()>0) {
				if (getEntry(s) == Utils::ERROR) {
					error("bad entry in the grid-mapfile:", s );
					continue;
				} else {
					n++;
				}
				 if (usrOpts.fqan==false && usrOpts.dn==false) {
					entries.push_back(s);
				} else if (usrOpts.dn==true &&
					Converter::checkCredentialType(s) == GaclManager::WMPGACL_PERSON_TYPE){
					entries.push_back(s);
				} else if (usrOpts.fqan==true &&
					Converter::checkCredentialType(s) == GaclManager::WMPGACL_VOMS_TYPE){
					entries.push_back(s);
				}
			}
		}
		in.close();
	} else {
		errors = "unable to read the grid-mapfile: " + path ;
		result = Utils::ERROR;
	}
	this->readEntries = entries.size() ;
	os1 << "Number of entries loaded from the grid-mapfile: " << n ;
	LOG (WMS_DEBUG, os1.str(), "");
	if (usrOpts.dn == true) {
		os2 << "the --only-dn option has been specified; number of dn-entries into the grid-mapfile:" << this->readEntries;
		LOG (WMS_DEBUG, os2.str(), "" );
	} else if (usrOpts.fqan == true) {
		os2 << "the --only-fqan option has been specified; number of fqan-entries into the grid-mapfile:" << this->readEntries;
		LOG (WMS_DEBUG, os2.str(), "" );
	}
	return result;
}
/**
*  Yes/No-question
*/
bool Converter::answerYes (const std::string& question, bool defaultAnswer, bool defaultValue) {
	if (usrOpts.noint){
		// No interaction required
		return defaultValue ;
	}
	string possible=" [y/n]";
	possible +=(defaultAnswer?"y":"n");
	possible +=" :";
	char x[1024];
	char *c = (char*)malloc(128);
	while (1){
		cout << question << possible << " " ;
		cin.getline(x,128);
		c=&x[0]; //cut off the \n char
		if((*c=='y')||(*c=='Y')){return true;}
		else if((*c=='n')||(*c=='N')){return false;}
		else if (*c=='\0'){return defaultAnswer;}
	}
	if(c){free(c);}
}
/**
* Setting up of the gacl file to be created
*/
void Converter::setUpGaclFile (const std::string &path,
	const bool& create,
	std::vector<std::string> &dns,
	std::vector<std::string> &fqans) {
	// checks whether the gacl file already exists
	this->existingGacl = GaclManager::gaclExists(path);
	// not replace the file !!!
	if (this->existingGacl && create == false) {
		this->newGacl = false;
		LOG(WMS_INFO, "Updating the gacl file:", path);
		usrGacl = new GaclManager (path);
		// if usrOpts.dn = false && usrOpts.fqan = false : updating of both credential type entries
		// if usrOpts.dn = true -> only dn-entries ; if usrOpts.fqan = true -> only fqan-entries
		// ( both flags are not allowed to be true at the same time)
		if (usrOpts.dn == false) {
			fqans = usrGacl->getItems(GaclManager::WMPGACL_VOMS_TYPE );
		} else if (usrOpts.fqan == false) {
			dns = usrGacl->getItems(GaclManager::WMPGACL_PERSON_TYPE );
		}
	} else
	// --new option (and the file already exists)
	if (this->existingGacl && create) {
		cout << "Gacl file path: " << path << "\n";
		//const string quest = "The file already exists. Do you wish to overwrite it ?";
		if (answerYes ("The file already exists. Do you wish to overwrite it ?", false, false) == false) {
			cout << "bye\n";
			exit(0);
		} else {
			LOG(WMS_INFO, "Creating a new gacl file:", path);
			this->newGacl = true;
			usrGacl = new GaclManager (path, this->newGacl);
		}
	} else if (create == false && this->existingGacl == false) {
		LOG(WMS_INFO, "Creating a new gacl file:", path);
		this->newGacl = true;
		usrGacl = new GaclManager (path, this->newGacl);
	}
}
/**
* Adds the entries that are in the grid-mapfile to the gacl file (if they are not already present)
*/
void Converter::addEntries (const std::vector<std::string> &entries,const glite::wms::wmproxy::authorizer::GaclManager::WMPgaclCredType &credential ) {
	string single = "";
	int size = 0;
	if (usrGacl) {
		size = entries.size( );
		for (int i = 0; i < size; i++) {
			single = entries[i] ;
			LOG(WMS_DEBUG, "Processing this line of the grid-mapfile:", single);
			GaclManager::WMPgaclCredType cred_type = Converter::checkCredentialType(single);
			if (credential == GaclManager::WMPGACL_UNDEFCRED_TYPE ||
				 cred_type == credential){
				if (usrGacl->hasEntry(cred_type, single)) {
					LOG(WMS_DEBUG,
						"Credential entry already present in the gacl file ("+ usrGacl->getCredentialTypeString(cred_type) + " type):",
						single);
						notReplacedEntries++;
					if (usrGacl->checkAllowPermission(cred_type, single, GRST_PERM_EXEC)==false){
						LOG(WMS_DEBUG, "Enable the exec permission for the credential previously found in the gacl",
							"(it was not allowed)");
						usrGacl->allowPermission(cred_type, single, GRST_PERM_EXEC);
					}
				} else {
					usrGacl->addEntry(cred_type, single, GRST_PERM_EXEC);
						LOG(WMS_DEBUG,
							"Adding a new entry in the gacl file ("+ usrGacl->getCredentialTypeString(cred_type) + " type):",
							single);
					// counter updating
					this->addedEntries++;
				}
			}
		}
	} else {
		error ("Converter::addEntries> Null pointer:", "gaclUsr attribute");
	}
}
/**
* Removes from the gacl file the entries that are not present in the grid-mapfile
*/
void Converter::removeOldEntries(const std::vector<std::string> &map, std::vector<std::string> &gacl, const glite::wms::wmproxy::authorizer::GaclManager::WMPgaclCredType &credential) {
	string gacl_entry = "";
	string map_entry = "";
	string e = "";
	string err = "";
	bool found = false;
	int gacl_size = gacl.size ( );
	for (int i = 0; i < gacl_size; i++) {
		gacl_entry = gacl[i];
		found = false;
		for (unsigned int j=0; j < map.size(); j++) {
			map_entry = map[j];
			GaclManager::WMPgaclCredType cred_type = Converter::checkCredentialType(map_entry);
			if (cred_type == credential){
				if (cred_type == GaclManager::WMPGACL_PERSON_TYPE &&
					WMPAuthorizer::compareDN((char*)map_entry.c_str(), (char*)gacl_entry.c_str())==0) {
					found = true;
					break;
				} else if (cred_type == GaclManager::WMPGACL_VOMS_TYPE &&
					WMPAuthorizer::compareFQAN((char*)map_entry.c_str(), (char*)gacl_entry.c_str())==true) {
					found = true;
					break;
				}
			}
		}
		if (found==false) {
			LOG(WMS_DEBUG, "Removing the following entry from the gacl file because it is not present in the grid-mapfile:\n",
			gacl_entry + "(" + usrGacl->getCredentialTypeString(credential) +")");
			if( usrGacl->removeEntry(credential, gacl_entry, err) < 0) {
				LOG(WMS_WARNING,
					"Removal operation failed:",
					err );
			} else {
				gacl.erase(gacl.begin()+i);
				i--;
				gacl_size = gacl.size( );
				LOG(WMS_DEBUG,
					"Gacl entry successfully removed",
					"" );
				// counter updating
				this->removedEntries++;
			}
		}
	}
}

/**
* Saves the gacl struct (stored in the memory) to the output file
*/
int Converter::saveGacl ( ) {
	int result = 0;
	if (usrGacl == NULL) {
		error("Converter::saveGacl> Null pointer:", "gaclUsr attribute");
		result = -1;
	} else {
		if (this->newGacl && this->addedEntries > 0) {
			if (this->existingGacl) {
				LOG(WMS_DEBUG, "Replacing the old gacl file with the new one:", usrGacl->getFilepath( )  );
			} else {
				LOG(WMS_DEBUG, "Saving the new gacl file:", usrGacl->getFilepath( )  );
			}
			result = usrGacl->saveGacl ( );

		} else if (this->addedEntries > 0 || this->removedEntries > 0) {
			LOG(WMS_DEBUG, "Saving the gacl file with changes:", usrGacl->getFilepath( )  );
			result = usrGacl->saveGacl ( );
		} else {
			if (this->existingGacl) {
				LOG(WMS_DEBUG, "No changes done into the gacl file", "");
			} else {
				LOG(WMS_DEBUG, "The gacl file has not been created:", "no grid-mapfile entries to be added to");
			}
		}
	}
	return result ;
}
/**
* Returns the credential type
*/
glite::wms::wmproxy::authorizer::GaclManager::WMPgaclCredType Converter::checkCredentialType (const std::string &raw) {
	// Personal certificate: it has to contain this string in any position
	if (raw.find(Utils::PERSONAL_FIELD) != string::npos ){
		return GaclManager::WMPGACL_PERSON_TYPE;
	} else
	// DNLIST if starts with http protocol
	if (raw.find(Utils::DNLIST_FIELD)== 0){
		return GaclManager::WMPGACL_DNLIST_TYPE;
	} else {
		return GaclManager::WMPGACL_VOMS_TYPE;
	}
}
std::string Converter::entries (int n, bool new_word) {
	ostringstream out ;
	string plural = "";
	string singular = "";
	if (usrOpts.dn) {
		singular = "dn-entry";
		plural = "dn-entries";
	} else if (usrOpts.fqan) {
		singular = "fqan-entry";
		plural = "fqan-entries";
	} else {
		singular = "valid entry";
		plural = "valid entries";
	}
	if (n==0) {
		if (new_word) { out << "new ";}
		out <<  plural;
	} else {
		out << n << " ";
		if (new_word) { out << " new ";}
		(n>1) ? out << plural : out << singular ;
	}
	return out.str( );

}
std::string Converter::getResultMsg ( ) {
	ostringstream out ;
	out << "===================================================================================\n\n";
	out << "\t\tGridMapFile2Gacl results\n\n";
	if (this->readEntries == 0) {
		out << "The following grid-mapfile does not contain any " << entries(0) << ":\n";
		out << this->gridMapFile <<"\n\n";
		if (this->existingGacl) {
			if (this->newGacl) {
				out << "The existing gacl file has not been overwritten:\n";
			} else {
				out << "The existing gacl file has not been changed:\n";
			}
		} else {
			out << "The gacl file has not been created:\n";
		}
		out << usrGacl->getFilepath( ) << "\n";
	} else {
		out << "The following grid-mapfile contains " << entries(this->readEntries) << ":\n";
		out << this->gridMapFile <<"\n\n";
		if (usrOpts.add)  {
			out << "Only adding of new " << entries(0) << " to the gacl file was allowed.\n\n";
		} else if (usrOpts.remove)  {
			out << "Only removal of new " << entries(0) << " from the gacl file was allowed.\n\n";
		}
		if (newGacl) {
			if (addedEntries == 0) {
				out << "No " << entries(0) << " into the grid-mapfile to be added to the gacl file;\n";
				out << usrGacl->getFilepath( ) << "\n";
				if(this->existingGacl) {
					out << "(the gacl file has not been overwritten)\n";
				} else {
					out << "(the gacl file has not been created)\n";
				}
			} else {
				out << "The following gacl file has been successfully created:\n";
				out << usrGacl->getFilepath( ) << "\n";
				out << "(with " << entries(this->addedEntries, true)<< ")\n";
			}
		} else {
			if (addedEntries == 0 && removedEntries == 0) {
				out << "No changes done in the following gacl file:\n";
				out << usrGacl->getFilepath( ) << "\n";
 				if (usrOpts.remove)  {
					out << "(none of the " << entries(0) << " in the grid-mapfile has been found in the gacl file)\n";
				} else {
					out << "(it already contains all the " << entries(0) << " of the grid-mapfile)\n";
				}

			} else {
				out << "The following gacl file has been successfully updated:\n";
				out << usrGacl->getFilepath( ) << "\n";
				if (notReplacedEntries > 0) {
					out << " - number of " <<  entries(0) << " already present: " << notReplacedEntries<< "\n";
				}
				if (addedEntries > 0) {
					out << " - number of " << entries(0, true) << ": " << this->addedEntries << "\n";
				}
				if (this->removedEntries > 0) {
					out << " - number of removed " <<  entries(0) << " (not present in the gridmap file): " << this->removedEntries << "\n";
				}
			}
		}
	}
	out << "\n===================================================================================\n";

	if (this->logFile) {
		string *log = logFile->getPathName( );
		if (log) {
			out << "\nPossible Errors and Debug messages have been printed in the following file:\n";
       			out << *log << "\n";
		}
	}
	return out.str( );
}

}}}} // ending namespaces

/**
* ===== main =======================================
*/

using namespace glite::wms::wmproxy::tools;


int main (int argc,char **argv){
	Converter converter;
	string msg = "";
	string err = "";
	string e = "";
	string output = "";
	int result = 0;
	int size = 0;
	vector<string> mapEntries;
	vector<string> dnEntries;
	vector<string> fqanEntries;
	Log *logFile = NULL;

	// reads the input options
	converterOpts *opts = converter.getUserOptions( );
	result = converter.readOptions (argc, argv, *opts, msg, err);
	// --version
	if (opts->version){
		cout << converter.version() << "\n\n";
		exit(0);
	} else
		// --help
		if (opts->help){
			converter.usage(string(argv[0]));
			exit(0);
		}
	if (opts->log){
		logFile = converter.createLogFile(*(opts->log));
	} else {
		logFile = converter.createLogFile( );
	}
	if (result == Utils::ERROR){
		converter.error(err, "");
	}
	LOG (WMS_DEBUG, msg, "");
	// pathname to outpur gacl file
	// Reading the Grid-mapfile
	if (converter.readMapFile (*(opts->input), mapEntries, err) == Utils::ERROR){
		converter.error(err, "");
		exit(-1);
	} else {
		size = mapEntries.size();
		if (size>0) {
			string e = "Grid-mapfile " + converter.entries(0) + ":\n";
			for (unsigned int i=0; i < mapEntries.size(); i++) {
				e += mapEntries[i] + "\n";
			}
			LOGFILE (WMS_INFO,e, "" );
		} else {
			LOG (WMS_DEBUG, "The grid-mapfile is empty", "");
		}
	}
	// opens the gacl file
	converter.setUpGaclFile(*(opts->output), opts->create,  dnEntries, fqanEntries) ;
	size = dnEntries.size( );
	if (size>0) {
		e = "dn-entries into the gacl file:\n";
		for (int i=0; i < size; i++) {
			e += dnEntries[i] + "\n";
		}
		LOGFILE(WMS_INFO, e, "");
	}
	size = fqanEntries.size( );
	if (size>0) {
		e = "fqan-entries into the gacl file:\n";
		for (int i=0; i < size; i++) {
			e += fqanEntries[i] + "\n";
		}
		LOGFILE(WMS_INFO, e, "");
	}
	if (opts->add){
		converter.addEntries(mapEntries);
	} else if (opts->dn) {
		if (converter.remove()) {
			// Remove the entries contained only in the old gacl file
			converter.removeOldEntries(mapEntries, dnEntries, GaclManager::WMPGACL_PERSON_TYPE);
		}
		if (converter.add( )) {
			// new entries
			converter.addEntries(mapEntries, GaclManager::WMPGACL_PERSON_TYPE);
		}
	} else  if (opts->fqan) {
		if (converter.remove( )) {
			// Remove the entries contained only in the old gacl file
			converter.removeOldEntries(mapEntries, fqanEntries, GaclManager::WMPGACL_VOMS_TYPE);
		}
		if (converter.add( )) {
			// Updates the entries
			converter.addEntries(mapEntries, GaclManager::WMPGACL_VOMS_TYPE);
		}
	} else {
		if (converter.remove( )) {
			// Remove the entries contained only in the old gacl file
			converter.removeOldEntries(mapEntries, dnEntries, GaclManager::WMPGACL_PERSON_TYPE);
			converter.removeOldEntries(mapEntries, fqanEntries, GaclManager::WMPGACL_VOMS_TYPE);
		}
		if (converter.add( )) {
			// Add  new entries
			converter.addEntries(mapEntries);
		}
	}
	// Saving the gacl file ...
	if (converter.saveGacl() < 0 ){
		converter.error("error while saving the gacl file", "");
		return -1;
	} else {
		// Result msg
		output = converter.getResultMsg ( ) ;
		LOGFILE(WMS_INFO, output, "");
		cout << output << "\n";
		return 0;
	}
};


