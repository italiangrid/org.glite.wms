


#ifndef GLITE_GACL_ADMIN
//Logger
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "utilities/logging.h"

// Exceptions
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"


//Utilities
#include "utilities/wmputils.h"

#endif

#include "wmpgaclmanager.h"
#include "wmpauthorizer.h"

#include <iostream>
#include <sstream>
#include <sys/stat.h>

namespace glite {
namespace wms {
namespace wmproxy {
namespace authorizer {

using namespace std;

#ifndef GLITE_GACL_ADMIN
using namespace glite::wms::wmproxy::utilities;
namespace logger = glite::wms::common::logger;
#endif

/*
	useful constants
*/

// permission codes
const unsigned int GaclManager::WMPGACL_NOPERM = GRST_PERM_NONE;
const unsigned int GaclManager::WMPGACL_READ	 = GRST_PERM_READ;
const unsigned int GaclManager::WMPGACL_EXEC = GRST_PERM_EXEC;
const unsigned int GaclManager::WMPGACL_LIST= GRST_PERM_LIST;
const unsigned int GaclManager::WMPGACL_WRITE = GRST_PERM_WRITE;
const unsigned int GaclManager::WMPGACL_ADMIN = GRST_PERM_ADMIN;

// user credentials
const char* GaclManager::WMPGACL_NONE_CRED  = "none";
const char* GaclManager::WMPGACL_ANYUSER_CRED = "any-user";
const char* GaclManager::WMPGACL_PERSON_CRED = "person" ;
const char* GaclManager::WMPGACL_DNLIST_CRED = "dn-list";
const char* GaclManager::WMPGACL_VOMS_CRED = "voms";
const char* GaclManager::WMPGACL_DNS_CRED = "dns";

// constants: credential tags
const char* GaclManager::WMPGACL_ANYUSER_TAG = "" ;
const char* GaclManager::WMPGACL_PERSON_TAG = "dn" ;
const char* GaclManager::WMPGACL_DNLIST_TAG = "url" ;
const char* GaclManager::WMPGACL_DNS_TAG = "hostname";
const char* GaclManager::WMPGACL_VOMS_TAG = "fqan";

// default filenames
const char* GaclManager::WMPGACL_DEFAULT_FILE = GRST_ACL_FILE;
const char* GaclManager::WMPGACL_DEFAULT_DRAIN_FILE = ".drain";

const int GaclManager::WMPGACL_SUCCESS = 0;
const int GaclManager::WMPGACL_ERROR = -1;

/*
 *
 * Constructor.
 * @param file path location of the gacl file
 *
 */
GaclManager::GaclManager (const string &file, const bool &create) {
	// file location
	gaclFile = gaclFile.assign(file) ;
	// init
	credType = "";
	gaclAcl = NULL ;
	gaclCred = NULL ;
	gaclUser = NULL ;
	gaclEntry = NULL;
	gaclAllowed = WMPGACL_NOPERM;
	gaclDenied =WMPGACL_NOPERM;
	if ( ! create ) {
		// checks if file exists
		if ( gaclExists (  ) ){
			// loads the existing file
			GRSTgaclInit();
			GaclManager::loadFromFile (gaclFile);
		} else {
			// exception: no such file
			std::ostringstream oss;
			oss <<  "gacl file not found (" << file << ")\n";
			#ifdef GLITE_GACL_ADMIN
			cerr << "Error : " << oss.str()<< endl;
			exit(-1);
			#else
			edglog(debug) << "Error : " << oss.str()<< endl;
			throw GaclException (__FILE__, __LINE__,  "GaclManager::GaclManager" ,
				WMS_GACL_ERROR,  oss.str() );
			#endif
		}
	} else {
		// creates a new file
		newGacl ( );
	}
};

/*
* Destructor
*/
GaclManager::~GaclManager( ) {
	#ifndef GLITE_GACL_ADMIN
	edglog_fn("GaclManager::~GaclManager");
	edglog(debug) << "cleaning memory by destructor"<< endl;
	#endif
	if (gaclCred) {delete (gaclCred);}
	if (gaclUser) {delete(gaclUser);}
	if (gaclEntry) {delete(gaclEntry);}
	if (gaclAcl) {delete(gaclAcl );}
	#ifndef GLITE_GACL_ADMIN
	edglog(debug) << "memory cleaned" << endl;
	#endif
};

/**
* Free the memory used by an existing ACL, and the memory used by any entries and credentials it may contain.
*/
void GaclManager::gaclFreeMemory( ) {
	#ifndef GLITE_GACL_ADMIN
	edglog_fn("GaclManager::gaclFreeMemory");
	edglog(debug) << "cleaning memory by gaclFreeMemory"<< endl;
	#endif
	if (gaclAcl) {
		#ifndef GLITE_GACL_ADMIN
		edglog(debug) << "cleaning acl"<< endl;
		#endif
		GRSTgaclAclFree(gaclAcl );
		#ifndef GLITE_GACL_ADMIN
		edglog(debug) << "memory cleaned" << endl;
		#endif
	} else {
		#ifndef GLITE_GACL_ADMIN
		edglog(debug) << "gacl is null" << endl;
		#endif
	}
};

/*
*	adds a credential entry to the gacl
*	@param type credential type
*	@param rawvalue credential raw (user-dn, fqan, etc...)
*/
void GaclManager::addEntry (const WMPgaclCredType &type, const string &rawvalue,const WMPgaclPerm &permission)
{
	// sets the credential attributes
	setCredentialType (type,rawvalue);
	// checks if the credential already exist into the file
	if ( loadCredential() ==WMPGACL_SUCCESS){
		std::ostringstream oss;
		oss <<  "unable to  add the new credential entry to the gacl" ;
		oss << " (" << gaclFile << ")\n";
		oss << "reason: the entry already exists\ncredential type : " << getCredentialTypeString(type)  << "\n";
		oss <<"rawvalue : " << rawvalue << "\n";
		#if defined (GLITE_GACL_ADMIN)
		cerr << "Error : " << oss.str()<< endl;
		exit(-1);
		#else
		edglog(debug) << "Error : " << oss.str() << endl;
  		throw GaclException (__FILE__, __LINE__,  "GaclManager::addEntry" ,
			WMS_GACL_ERROR,  oss.str() );
		#endif
	};
	// creates the new entry
	newCredential( );
	//sets permission
	setAllowPermission(type, rawvalue, permission);
};

bool GaclManager::hasEntry(const WMPgaclCredType &type, const string rawvalue){
	if (loadCredential(type,rawvalue)== WMPGACL_SUCCESS){
		return true ;
	} else{
		return false ;
	}


};
/*
*	adds a set of credential entries to the gacl
*	@param vect vector of credential (credential type , rawvalue )
*/
void GaclManager::addEntries (const vector<pair<WMPgaclCredType, string> > &vect )
{
	for (unsigned int i = 0; i < vect.size() ; i++){
		addEntry( (WMPgaclCredType)vect[i].first,
				(string)vect[i].second);
	}
};
/*
*	removes from the gacl file the entry of the specified credential
* 	@param file path location of the gacl file
*	@param type credential type
*	@param rawvalue credential raw (user-dn, fqan, etc...)
*	@param unset_perm true = reset user permission in the gacl
* 	@param create true = replace the gacl file with a new one
*/
void GaclManager::removeEntry (const WMPgaclCredType &type,
			const string rawvalue )
{
	#ifndef GLITE_GACL_ADMIN
	edglog_fn("GaclManager::removeEntry");
	#endif
	GRSTgaclCred  *cred = NULL;
	GRSTgaclCred  *prev_cred = NULL;
	GRSTgaclEntry *entry = NULL;
	GRSTgaclEntry *prev_entry = NULL;
	GRSTgaclNamevalue *nv = NULL;
	bool found = false;
	// sets attributes related to the type of creddential
	setCredentialType (type,rawvalue);
	char* rawname =(char*) rawCred.first.c_str();
	if (gaclAcl !=NULL) {
		//edglog(debug)<< "loadCredential> new entry" << rawValue<< endl ;
		entry = gaclAcl-> firstentry ;
		// scanning of the entries
		while  (entry != NULL ) {
			//edglog(debug)<<"entry not null" << endl;
			cred = entry->firstcred ;
			// scanning of the credentials in the selected entry
			while ( cred != NULL ) {
				// credType=WMPGACL_XXXXX_TAG (any-user, person, ......)
				#ifndef GLITE_GACL_ADMIN
				edglog(debug)<< "cred-type:" << cred->type<< endl;
				#endif
				if ( strcmp( cred->type, (char*)credType.c_str()) == 0 ){
					#ifndef GLITE_GACL_ADMIN
					edglog(debug)<< "entry found" <<endl;
					#endif
					//cout<< "entry found" <<endl;
					// comparison between input and gacl credentials:
					// nv =  < nv->name, nv->value>
					if ( strcmp((char*)credType.c_str(), GaclManager::WMPGACL_ANYUSER_CRED) == 0 ){
						found = true;
					} else {
						nv = cred->firstname;
						if (nv){
							//edglog(debug)<< "nv->name=" << nv->name << "/rawname=" << rawname <<endl;
							//edglog(debug)<< "nv->value=" << nv->value << "/rawvalue=" << rawvalue <<endl;
							if (strcmp (rawname, nv->name ) == 0 ){
								if ( strcmp((char*)credType.c_str(), GaclManager::WMPGACL_VOMS_CRED) == 0 ){
									#ifndef GLITE_GACL_ADMIN
									edglog(debug) << "comparison between voms fqan ..." << endl;
									#endif
									found = authorizer::WMPAuthorizer::compareFQAN( rawvalue, nv->value);
									edglog(debug) << "compareFQAN = " << found << "\n";
								} else{
									#ifndef GLITE_GACL_ADMIN
									edglog(debug) << "checking rawvalue ..." << endl;
									#endif
									if ( strcmp (rawvalue.c_str(), nv->value ) == 0){
										 edglog(debug) << "rawvalue found" << endl;
										found = true;
									}
									
								} // if(VOMS_CRED)
							} // if (name)
						} // if (nv)
					} // if (ANY_CRED)
				}
				#ifndef GLITE_GACL_ADMIN
				edglog(debug) << "found=" << found << endl;
				#endif
				if ( !found ) {
					//edglog(debug) << "new credential....." << endl;
					prev_cred = cred ;
					cred = (GRSTgaclCred*) cred->next;
				} else {
					break;
				}
			} // while(cred)

			if ( !found ) {
				//edglog(debug) << "new entry ....." << endl;
				prev_entry = entry ;
				entry = (GRSTgaclEntry*) entry->next;
			} else {
				#ifndef GLITE_GACL_ADMIN
				edglog(debug) << "entry found : removing .....\n";
				#endif
				if (prev_entry) {
					prev_entry->next = (GRSTgaclEntry*) entry->next;
				} else {
					if ( entry->next ){
						gaclAcl-> firstentry = (GRSTgaclEntry*) entry->next;
					} else {
						gaclAcl = NULL ;
					}
				}
				GRSTgaclEntryFree (entry);
				break;
			}
		} //while(entry)
	} //if
	else{
		#ifndef GLITE_GACL_ADMIN
		edglog(debug) << "ACL is null" << "\n";
		#endif
	}
	if (!found){
		std::ostringstream oss;
		oss <<  "unable to remove the credential entry from the gacl" ;
		oss << " (" << gaclFile << ")\n";
		oss << "reason : the entry doesn't exist\ncredential type : " <<  getCredentialTypeString(type) << "\n";
		oss <<"rawvalue : " << rawvalue << "\n";
		#ifdef GLITE_GACL_ADMIN
		cerr << "Error : " <<  oss.str()<< endl;
		exit(-1);
		#else
		edglog(debug) << "Error : " <<  oss.str()<< endl;
  		throw GaclException (__FILE__, __LINE__,  "GaclManager::removeEntry" ,
			WMS_GACL_ERROR,  oss.str() );
		#endif
	};
};




/*
 * Checks if permission is allowed
 * @param permission permission  (WMPGACL_xxxxx)
 *
 */
bool GaclManager::checkAllowPermission (const WMPgaclCredType &type,
						const string &rawvalue,
						const WMPgaclPerm &permission) {
	bool allow = false;
	bool deny = false;
	bool result = -1;
	string errmsg = "";
	// load Credential
	if (  loadCredential(type, rawvalue) != WMPGACL_SUCCESS){
		std::ostringstream oss;
		oss <<  "unable to check credential permission " ;
		oss << " (" << gaclFile << ")\n";
		oss << "(credential entry not found)\ncredential type: " << getCredentialTypeString(type)  << "\n";
		oss <<"rawvalue : " << rawvalue << "\n";
		#ifdef GLITE_GACL_ADMIN
		cerr << "Error: " << oss.str()<< endl;
		exit(-1);
		#else
		edglog(debug) << oss.str()<< endl;
  		throw GaclException (__FILE__, __LINE__,  "GaclManager::checkAllowPermission" ,
			WMS_GACL_ERROR,  oss.str() );
		#endif
	};
	// permission
	allow = GaclManager::gaclAllowed & permission ;
	deny = GaclManager::gaclDenied & permission ;
	#ifndef GLITE_GACL_ADMIN
	edglog(debug)<<"checkAllowPermission> allow="<< allow << endl;
	#endif
	if ( allow && deny ) {
		errmsg = "gacl syntax error: operation both allowed and denied (" + gaclFile +")" ;
		#ifdef GLITE_GACL_ADMIN
		cerr << "Error: " << errmsg << endl;
		exit(-1);
		#else
		edglog(debug) << errmsg << endl;
  		throw GaclException (__FILE__, __LINE__,  "GaclManager::checkAllowPermission" ,
			WMS_GACL_ERROR, errmsg );
		#endif
	 } else  {
	 	if ( !allow && !deny ) {
			result = false;
		 } else {
			result = (int) allow ;
		 }
	 }
	 #ifndef GLITE_GACL_ADMIN
	edglog(debug)<< "GaclManager::checkAllowPermission> result=" <<result << endl;
	#endif
	return result ;
 };

/*
 * Checks if permission is denied
 */
bool GaclManager::checkDenyPermission (const WMPgaclCredType &type,
	const string &rawvalue, const WMPgaclPerm &permission) {
	string errmsg = "";
	if (loadCredential(type, rawvalue) != WMPGACL_SUCCESS) {
		return false;
	};
	return GaclManager::gaclDenied & permission;

 };

/*
 * allows permission to the user
 * @param permission permission  (WMPGACL_xxxxx)
*/
void GaclManager::allowPermission(const WMPgaclCredType &type,
						const string &rawvalue,
						const WMPgaclPerm &permission,
						const bool &unset_perm) {

	// load Credential
	if (  loadCredential(type, rawvalue) != WMPGACL_SUCCESS){
		std::ostringstream oss;
		oss <<  "unable to set \"allow\" permission" ;
		oss << " (" << gaclFile << ")\n";
		oss << "reason : credential entry not found\ncredential type: " << getCredentialTypeString(type) << "\n";
		oss <<"rawvalue : " << rawvalue << "\n";
		#ifdef GLITE_GACL_ADMIN
		cerr << "Error : "<< oss.str()<< endl;
		exit(-1);
		#else
		edglog(debug) << "Error : "<< oss.str()<< endl;
  		throw GaclException (__FILE__, __LINE__,  "GaclManager::allowPermission" ,
			WMS_GACL_ERROR,  oss.str() );
		#endif
	};
	// reset permission if it is requested
	if (unset_perm){
		gaclAllowed =GaclManager::WMPGACL_NOPERM ;
		gaclDenied =  gaclEntry->denied;
	}
	// undenies and allows "permission"
	GRSTgaclEntryAllowPerm (gaclEntry, permission) ;
	GRSTgaclEntryUndenyPerm (gaclEntry, permission) ;
	gaclAllowed = gaclEntry->allowed ;
	gaclDenied =  gaclEntry->denied;
 };

/*
 * denies permission to the user
 * @param permission permission  (WMPGACL_xxxxx)
*/
void GaclManager::denyPermission(const WMPgaclCredType &type,
						const string &rawvalue,
						const WMPgaclPerm &permission,
						const bool &unset_perm	) {
	if (  loadCredential(type, rawvalue) != WMPGACL_SUCCESS){
		std::ostringstream oss;
		oss <<  "unable to set \"deny\" permission" ;
		oss << " (" << gaclFile << ")\n";
		oss << "reason: credential entry not found\ncredential type: " << getCredentialTypeString(type) << "\n";
		oss <<"rawvalue : " << rawvalue << "\n";
		#ifdef GLITE_GACL_ADMIN
		cerr << "Error: " << oss.str()<< endl;
		exit(-1);
		#else
		edglog(debug) << oss.str()<< endl;
  		throw GaclException (__FILE__, __LINE__,  "GaclManager::denyPermission" ,
			WMS_GACL_ERROR,  oss.str() );
		#endif
	};
	// reset permission if it is requested
	if (unset_perm){
		gaclAllowed =GaclManager::WMPGACL_NOPERM ;
		gaclDenied =  gaclEntry->denied;
	}
	// denies and unallows "permission"
	GRSTgaclEntryDenyPerm (gaclEntry, permission) ;
	GRSTgaclEntryUnallowPerm (gaclEntry, permission) ;
	gaclAllowed = gaclEntry->allowed ;
	gaclDenied =  gaclEntry->denied;

 };
/*
 *	sets the allowed permission (it resets the old permission)
 * 	@param permission permission  (WMPGACL_xxxxx)
*/
void GaclManager::setAllowPermission(const WMPgaclCredType &type,
						const string &rawvalue,
						const WMPgaclPerm &permission) {

	#ifdef GLITE_GACL_ADMIN
	// calls "allowPermission" with the unset_perm = true
	allowPermission(type, rawvalue, permission, true);
	#else
	try {
		// calls "allowPermission" with the unset_perm = true
		allowPermission(type, rawvalue, permission,true);
	} catch (GaclException &exc) {
		std::ostringstream oss;
		oss << flush << exc.what() << endl;
  		throw GaclException (__FILE__, __LINE__,
				"GaclManager::setAllowPermission" ,
				WMS_GACL_ERROR,  oss.str() );
	}
	#endif
 };
/*
 *	sets the deniedpermission (it resets the old permission)
 * 	@param permission permission  (WMPGACL_xxxxx)
*/
void GaclManager::setDenyPermission(const WMPgaclCredType &type,
						const string &rawvalue,
						const WMPgaclPerm &permission) {

	#ifdef GLITE_GACL_ADMIN
	// calls "allowPermission" with the unset_perm = true
	denyPermission(type, rawvalue, permission, true);
	#else
	try {
		// calls "allowPermission" with the unset_perm = true
		denyPermission(type, rawvalue, permission, true);
	} catch (GaclException exc){
		std::ostringstream oss;
		oss << flush << exc.what() << endl;
  		throw GaclException (__FILE__, __LINE__,
				"GaclManager::setDenyPermission" ,
				WMS_GACL_ERROR,  oss.str() );
	}
	#endif
 };
/*
 * saves the gacl to the file
 * @param file location where to save the file
*/
int GaclManager::saveGacl (const string &file){
	int result = -1;
	if ( gaclAcl ) {
		result = GRSTgaclAclSave (gaclAcl, (char*)file.c_str() );
	} else {
		newGacl ( );
		GRSTgaclAclSave (gaclAcl, (char*)file.c_str() );
	}
	if ( result == 0) {
		return WMPGACL_ERROR;
	} else {
		return WMPGACL_SUCCESS;
	}
};

/*
 * saves the gacl in a file
*/
int GaclManager::saveGacl ( ){
	return saveGacl (gaclFile);
};
/*
* Gets the filepath
*/
std::string GaclManager::getFilepath ( ){
	return gaclFile;
};

/*
* Returns the string describing the type of credential
*/
std::string GaclManager::getCredentialTypeString ( const WMPgaclCredType &type){
	string str = "";
	switch (type){
		case WMPGACL_UNDEFCRED_TYPE:{
			str = GaclManager::WMPGACL_NONE_CRED ;
			break;
		}
		case WMPGACL_ANYUSER_TYPE:{
			str = GaclManager::WMPGACL_ANYUSER_CRED ;
			break;
		}
		case WMPGACL_PERSON_TYPE:{
			str = GaclManager::WMPGACL_PERSON_CRED ;
			break;
		}
		case WMPGACL_DNLIST_TYPE:{
			str = GaclManager::WMPGACL_DNLIST_CRED ;
			break;
		}
		case WMPGACL_VOMS_TYPE:{
			str = GaclManager::WMPGACL_VOMS_CRED ;
			break;
		}
		case WMPGACL_DNS_TYPE:{
			str = GaclManager::WMPGACL_DNS_CRED;
			break;
		}
		default:{
			str = "";
			break;
		}
	}
	return str;
};
/*
 * checks whether the gacl file exists
 * @param file the location of the gacl file
*/
 bool GaclManager::gaclExists ( ) {
	#ifndef GLITE_GACL_ADMIN
	edglog_fn("GaclManager::gaclExists");
	edglog(debug) << "checking file gacl existence" << endl;
	#endif
	struct stat buffer;
	if (stat(gaclFile.c_str(), &buffer)) {
		return 0;
	}
	return 1;
}
/*
*	adds a set of credential entries to the gacl
*	@param vect vector of credential (credential type , rawvalue )
*/
void GaclManager::removeEntries (const vector<pair<WMPgaclCredType, string> > &vect ){
	for (unsigned int i = 0; i < vect.size() ; i++){
		removeEntry( (WMPgaclCredType)vect[i].first,
				(string)vect[i].second);
	}
};
/*
*	============================
*	PRIVATE METHODS
*	============================
*/
/*
 *	loads the gacl from a file
 *	@param file path location of the gacl file
*/
void GaclManager::loadFromFile( const string &file ) {
	#ifndef GLITE_GACL_ADMIN
	edglog_fn("GaclManager::loadFromFile");
	edglog(debug) << "loading gacl from file : [" << file << "]" << endl;
	#endif
	gaclAcl = GRSTgaclAclLoadFile ( (char*) file.c_str() );
	if (gaclAcl == NULL) {
		#ifdef GLITE_GACL_ADMIN
		cerr << "Error: unable to load gacl from file (" << file << ")\n";
		exit(-1);
		#else
		edglog(debug) << "gacl file not loaded: gaclAcl is null" << endl;
		ostringstream oss ;
		oss << "unable to load gacl from file : [" << file << "] (contact the server administrator)";
		edglog(debug) << oss.str() << endl;
		throw GaclException (__FILE__, __LINE__,  "GaclManager::GaclManager" ,
			WMS_GACL_ERROR,  oss.str() );
		#endif
	} else {
		#ifndef GLITE_GACL_ADMIN
		edglog(debug) << "the gacl has been successfully stored" << endl;
		#endif
	}
};
/*
 * creates new gacl
 *
*/
void GaclManager::newGacl (){
	gaclCred = NULL ;
	gaclUser = NULL ;
	// gacl init ....
	GRSTgaclInit ( ) ;
	gaclAcl = GRSTgaclAclNew ();
	if ( GaclManager::gaclAcl == NULL ){
		string errmsg = "Fatal error: unable to create a new gacl";
		#ifdef GLITE_GACL_ADMIN
		cerr << "Error: " << errmsg << endl;
		exit(-1);
		#else
		edglog(debug) << errmsg << endl;
		throw GaclException (__FILE__, __LINE__,  "newGacl( )" ,
			WMS_GACL_ERROR, errmsg);
		#endif
	}
}
const vector<string> GaclManager::getItems(const WMPgaclCredType &type){
	#ifndef GLITE_GACL_ADMIN
	edglog_fn("GaclManager::loadCredential");
	#endif
	GRSTgaclCred  *cred = NULL;
	GRSTgaclEntry *entry = NULL;
	GRSTgaclNamevalue *nv = NULL;
	vector<string> items ;
	//operation not allow for any-user credential
	if (type == WMPGACL_ANYUSER_TYPE){
		string errmsg = "operation not allows for any-user credential ";
		errmsg = "reason: no identifiers for this type of credential\n";
		#ifdef GLITE_GACL_ADMIN
		cerr << "Error: " << errmsg << endl;
		exit(-1);
		#else
		edglog(debug) << "Error: " << errmsg << endl;
		throw GaclException (__FILE__, __LINE__,  "getItems( )" ,
			WMS_GACL_ERROR, errmsg);
		#endif
	}
	//sets credential type
	setCredentialType(type, "");
	// credential name
	char* rawname =(char*) rawCred.first.c_str();
	// looks for the specified credential
	if (gaclAcl !=NULL) {
		//edglog(debug)<< "loadCredential> new entry" << rawValue<< endl ;
		entry = gaclAcl-> firstentry ;
		// scanning of the entries
		while  (entry != NULL ) {
			//edglog(debug)<<"entry not null" << endl;
			cred = entry->firstcred ;
			// scanning of the credentials in the selected entry
			while ( cred != NULL ) {
				// credType=WMPGACL_XXXXX_TAG (any-user, person, ......)
				#ifndef GLITE_GACL_ADMIN
				edglog(debug)<< "cred-type:" << cred->type<< endl;
				#endif
				if ( strcmp( cred->type, (char*)credType.c_str()) == 0 ){
					#ifndef GLITE_GACL_ADMIN
					edglog(debug)<< "entry found" <<endl;
					#endif
					// comparison between input and gacl credentials:
					// nv =  < nv->name, nv->value>
					nv = cred->firstname;
					if (nv && nv->name){
						//edglog(debug)<< "nv->name=" << nv->name << "/rawname=" << rawname <<endl;
						//edglog(debug)<< "nv->value=" << nv->value << "/rawvalue=" << rawvalue <<endl;
						if (strcmp (rawname, nv->name ) == 0 ){
							if (nv->value){
								items.push_back (nv->value);
							}
						} // if (name)
					} // if (nv)
				} // if(strcmp)
				cred = (GRSTgaclCred*) cred->next;
			} // while(cred)
			// next entry .
			entry = (GRSTgaclEntry*) entry->next;
		} //while(entry)
	}

	return items ;
}
/*
	sets the internal class attributes related to
	the credential
*/
void GaclManager::setCredentialType(const WMPgaclCredType &type, const string &rawvalue)
{
	switch ( type ){
		case WMPGACL_ANYUSER_TYPE : {
				credType =GaclManager::WMPGACL_ANYUSER_CRED ;
				rawCred = make_pair(GaclManager::WMPGACL_ANYUSER_TAG, "");
				break;
		}
		case WMPGACL_PERSON_TYPE: {
				credType = GaclManager::WMPGACL_PERSON_CRED ;
				rawCred = make_pair(GaclManager::WMPGACL_PERSON_TAG, rawvalue);
				break;
		}
		case  WMPGACL_DNLIST_TYPE: {
				credType = GaclManager::WMPGACL_DNLIST_CRED ;
				rawCred = make_pair(GaclManager::WMPGACL_DNLIST_TAG, rawvalue);
				break;
		}
		case  WMPGACL_VOMS_TYPE: {
				credType = GaclManager::WMPGACL_VOMS_CRED ;
				rawCred = make_pair(GaclManager::WMPGACL_VOMS_TAG, rawvalue);
				break;
		}
		case  WMPGACL_DNS_TYPE: {
				credType = GaclManager::WMPGACL_DNS_CRED ;
				rawCred = make_pair(GaclManager::WMPGACL_DNS_TAG, rawvalue);
				break;
		}
		default : {
			string errmsg = "credential type not supported";
			#ifdef GLITE_GACL_ADMIN
			cerr << "Error: " << errmsg << endl;
			exit(-1);
			#else
			edglog(debug) << errmsg << endl;
			throw GaclException(__FILE__, __LINE__,
	    			"GaclManager::setCredentialType",
	    			WMS_GACL_ERROR, errmsg);
			#endif
	    	break;
		}
	}
};

int GaclManager::loadCredential ( const WMPgaclCredType &type,
					const string &rawvalue)
{
	#ifndef GLITE_GACL_ADMIN
	edglog_fn("GaclManager::loadCredential");
	#endif
	// sets attributes related to the type of creddential
	setCredentialType (type,rawvalue);
	return loadCredential( );
};

/*
 * loads credential
*/
int GaclManager::loadCredential ( ) {
	#ifndef GLITE_GACL_ADMIN
	edglog_fn("GaclManager::loadCredential");
	#endif
	GRSTgaclCred  *cred = NULL;
	GRSTgaclEntry *entry = NULL;
	GRSTgaclNamevalue *nv = NULL;
	bool found = false;
	char* rawname =(char*) rawCred.first.c_str();
	char* rawvalue =(char*)  rawCred.second.c_str();

	if (gaclAcl !=NULL) {
		//edglog(debug)<< "loadCredential> new entry" << rawValue<< endl ;
		entry = gaclAcl-> firstentry ;
		// scanning of the entries
		while  (entry != NULL ) {
			//edglog(debug)<<"entry not null" << endl;
			cred = entry->firstcred ;
			// scanning of the credentials in the selected entry
			while ( cred != NULL ) {
				// credType=WMPGACL_XXXXX_TAG (any-user, person, ......)
				#ifndef GLITE_GACL_ADMIN
				edglog(debug)<< "cred-type:" << cred->type<< endl;
				#endif
				if ( strcmp( cred->type, (char*)credType.c_str()) == 0 ){
					#ifndef GLITE_GACL_ADMIN
					edglog(debug)<< "entry found" <<endl;
					#endif
					// comparison between input and gacl credentials:
					// nv =  < nv->name, nv->value>
					if ( strcmp((char*)credType.c_str(), GaclManager::WMPGACL_ANYUSER_CRED) == 0 ){
						found = true;
					} else {
						nv = cred->firstname;
						if (nv){
							//edglog(debug)<< "nv->name=" << nv->name << "/rawname=" << rawname <<endl;
							//edglog(debug)<< "nv->value=" << nv->value << "/rawvalue=" << rawvalue <<endl;
							if (strcmp (rawname, nv->name ) == 0 ){
								if ( strcmp((char*)credType.c_str(), GaclManager::WMPGACL_VOMS_CRED) == 0 ){
									#ifndef GLITE_GACL_ADMIN
									edglog(debug) << "comparison between voms fqan ..." << endl;
									#endif
									found = authorizer::WMPAuthorizer::compareFQAN( rawvalue, nv->value);
								} else {
									#ifndef GLITE_GACL_ADMIN
									edglog(debug) << "checking rawvalue ..." << endl;
									#endif
									// found = authorizer::WMPAuthorizer::compareDN(rawvalue, nv->value);
									if ( strcmp (rawvalue, nv->value ) == 0){
										found = true;
									}
								} // if(VOMS_CRED)
							} // if (name)
						} // if (nv)
					} // if (ANY_CRED)
				}
				#ifndef GLITE_GACL_ADMIN
				edglog(debug) << "found=" << found << endl;
				#endif
				if ( !found ) {
					//edglog(debug) << "new credential....." << endl;
					cred = (GRSTgaclCred*) cred->next;
				} else {
					break;
				}
			} // while(cred)

			if ( !found ) {
				//edglog(debug) << "new entry ....." << endl;
				entry = (GRSTgaclEntry*) entry->next;
			} else {
				break;
			}
		} //while(entry)
	} //if
	else{
		#ifndef GLITE_GACL_ADMIN
		edglog(debug) << "ACL is null" << "\n";
		#endif
	}

	if (entry != NULL ) {
		// main pointers
		gaclEntry = entry ;
		gaclCred = cred ;
		gaclUser  = GRSTgaclUserNew (cred);
		// allowed and denied operations currently set
		gaclAllowed = entry->allowed ;
		gaclDenied = entry->denied;
	}
	// resturn values
	if (found){
		return WMPGACL_SUCCESS ;
	} else {
		return WMPGACL_ERROR ;
	}
};
/*
 * creates the credential
*/
void GaclManager::newCredential ( )
{
	string errmsg = "";
	// <rawname><rawvalue></rawname>
	char* rawname =(char*) rawCred.first.c_str();
	char* rawvalue =(char*)  rawCred.second.c_str();
	// new acl
	if ( gaclAcl  == NULL ){
		GRSTgaclInit ( ) ;
		gaclAcl = GRSTgaclAclNew ();
	}
	if ( gaclAcl == NULL ){
		errmsg = "Fatal error: unable to create new gacl";
		#ifdef GLITE_GACL_ADMIN
		cerr << "Error: " << errmsg << endl;
		exit(-1);
		#else
		edglog(debug) << errmsg << endl;
		throw GaclException (__FILE__, __LINE__,  "newCredential (WMPgaclCredType,string, string, string)" ,
			WMS_GACL_ERROR, errmsg);
		#endif
	}
	// new entry
	gaclEntry = GRSTgaclEntryNew ( );
	if ( gaclEntry == NULL ){
		errmsg ="Fatal error; unable to create a new gacl entry";
		#ifdef GLITE_GACL_ADMIN
		cerr << "Error: " << errmsg << endl;
		exit(-1);
		#else
		edglog(debug) << errmsg << endl;
		throw GaclException (__FILE__, __LINE__,  "newCredential (WMPgaclCredType, string, string, string)" ,
				WMS_GACL_ERROR, errmsg);
		#endif
	}
	gaclCred = GRSTgaclCredNew ((char *)credType.c_str());
	if ( strcmp((char*)credType.c_str(), GaclManager::WMPGACL_ANYUSER_CRED) != 0 )
	 	GRSTgaclCredAddValue(gaclCred,rawname, rawvalue);
	if ( gaclCred == NULL ){
		errmsg ="Fatal error: unable to create new credential";
		#ifdef GLITE_GACL_ADMIN
		cerr << "Error: " << errmsg << endl;
		exit(-1);
		#else
		edglog(debug) << errmsg << endl;
		throw GaclException (__FILE__, __LINE__,  "newCredential (string, string, string)" ,
			WMS_GACL_ERROR, errmsg );
		#endif
	}
	// new user
	gaclUser  = GRSTgaclUserNew (gaclCred) ;
	if ( gaclUser == NULL ){
		errmsg ="Fatal error: unable to create new user credential";
		#ifdef GLITE_GACL_ADMIN
		cerr << "Error: " << errmsg << endl;
		exit(-1);
		#else
		edglog(debug) << errmsg << endl;
		throw GaclException (__FILE__, __LINE__,  "newCredential (string, string, string)" ,
			WMS_GACL_ERROR, errmsg);
		#endif
	}
	// adds the created credential to the entry
	GRSTgaclEntryAddCred (gaclEntry, gaclCred);
	// adds the entry to the gacl
	GRSTgaclAclAddEntry (gaclAcl, gaclEntry);
};
} // namespace authorizer
} // namespace wmproxy
} // namespace wms
} // namespace glite

