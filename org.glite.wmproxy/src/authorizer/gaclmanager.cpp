


//Logger
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "logging.h"

#include "gaclmanager.h"

using namespace std;
using namespace glite::wmsutils::exception; //Exception

namespace logger       = glite::wms::common::logger;

WMPgaclPerm perm_vect[ ] ={
	WMPGACL_NOPERM,
	WMPGACL_READ	,
	WMPGACL_EXEC	,
	WMPGACL_LIST	,
	WMPGACL_WRITE	,
	WMPGACL_ADMIN
};
GaclManager::GaclManager ( ) {};
/*
 * Constructor.
 * @param file path location of the gacl file
 * @param create true = create the file if it doesn't exist (default value)
 */

GaclManager::GaclManager (string file, WMPgaclCredType type, string dn, bool unset_perm, bool create) {

	gaclAcl = NULL ;
	gaclCred = NULL ;
	gaclUser = NULL ;

	loadNewCredential ( file, type, dn, unset_perm, create);

	edglog(debug)<<"end constructor"<< endl;
}

/*
	CHECK-METHODS

*/
/*
 * Checks if the permission is allowed to user
 * @user username
 * @param permission permission  (WMPGACL_xxxxx)
 * @return 1 if the permission is allowed ; 0 is not allowed ;
 *               -1 in case of bad gacl format (both allow and deny are set)
 *		-2 in case of bad gacl format (neither allow or deny is set)
 */
int GaclManager::checkAllowPermission (WMPgaclPerm permission) {

	bool allow, deny = false;
	int result = -1;

	edglog(debug)<<"checkAllowPermission"<< endl;

	// permission
	allow = ! (! (GaclManager::gaclAllowed & permission)  );

	deny = ! (! (GaclManager::gaclDenied & permission)  );


	if ( allow && deny ) {
		edglog(debug)<< "both allow and deny are set!!!" << endl;
		result -1 ;
	 } else  {
	 	if ( !allow && !deny ) {
	 		edglog(debug) << "neither allow or deny is  set!!!" << endl;
			result = -2;
		 } else {
			result = (int) allow ;
		 }
	 }
	 edglog(debug)<< "GaclManager::checkAllowPermission> result=" <<result << endl;

	return result ;
 };


 int GaclManager::allowPermission(WMPgaclPerm permission) {

	int result = -1 ;

	edglog(debug)<<"allowPermission start"<< endl;

	if (gaclEntry != NULL  ) {

		GRSTgaclEntryAllowPerm (gaclEntry, permission) ;

		GRSTgaclEntryUndenyPerm (GaclManager::gaclEntry, permission) ;

		gaclAllowed = gaclEntry->allowed ;
		gaclDenied =  gaclEntry->denied;

	}

	return result ;

 };

/*
 * denies permission to the user
 * @user user name
 * @param permission permission  (WMPGACL_xxxxx)
*/
 int GaclManager::denyPermission(WMPgaclPerm permission) {

	int result = -1;
	edglog(debug)<<"denyPermission start"<< endl;

	if (gaclEntry != NULL  ) {

		GRSTgaclEntryDenyPerm (GaclManager::gaclEntry, permission) ;

		GRSTgaclEntryUnallowPerm (GaclManager::gaclEntry, permission) ;

		gaclAllowed = gaclEntry->allowed ;
		gaclDenied =  gaclEntry->denied;
	}

	return result ;
 };


/*
 * saves the gacl to the file
 * @param file location where to save the file
*/
int GaclManager::saveGacl ( string file ){

	int result = GRSTgaclAclSave (GaclManager::gaclAcl, (char*)file.c_str() );

	if ( result == 0)
		return WMPGACL_ERROR;
	else
		return WMPGACL_SUCCESS;
};

int GaclManager::saveGacl ( ){

	int result = GRSTgaclAclSave (GaclManager::gaclAcl, (char*)gaclFile.c_str());

	if ( result == 0)
		return WMPGACL_ERROR;
	else
		return WMPGACL_SUCCESS;
};
/*
 * loads the gacl to the file
 * @param file the location of the file
*/
void GaclManager::loadFromFile( string file ) {

	GaclManager::gaclAcl = GRSTgaclAclLoadFile ( (char*) file.c_str() );
	/*
	if ( gaclAcl == NULL ) {
		throw GaclException (__FILE__, __LINE__,  "GaclManager::loadFromFile" , -1, "unable to load gacl file: [" + file +"]" );
	}
	*/

};

/*
 * prints the gacl
*/
void GaclManager::printGacl ( ) {

	if ( GaclManager::gaclAcl != NULL){
		GRSTgaclAclPrint (GaclManager::gaclAcl, stdout );
	} else
		edglog(info) << "no gacl loaded" << std::endl;

};

/*
 * checks whether the gacl file exists
 * @param file the location of the gacl file
*/
 bool GaclManager::gaclExists (string file) {

    FILE* fptr = NULL;
    bool exist = false;

    fptr = fopen ( (char*) file.c_str() , "r");
    if (fptr != NULL) {
    	exist = true;
	fclose (fptr);
    }

    return exist;
}

void GaclManager::newGacl (){


	gaclCred = NULL ;
	gaclUser = NULL ;

	GRSTgaclInit ( ) ;
	gaclAcl = GRSTgaclAclNew ();

	if ( GaclManager::gaclAcl == NULL ){
		throw GaclException (__FILE__, __LINE__,  "newGacl( )" , -1,
				"fatal error; unable to create new gacl ");
	}
}
/*
 * creates the credential for the user
 * @user username the user
 * @param rawname
 * @param rawvalue
*/
void GaclManager::newCredential ( ){

	int result = - 1;
	string errmsg = "";
	string cred_type = "";
	string cred_tag = "";

	// new acl
	if ( gaclAcl  == NULL ){
		GRSTgaclInit ( ) ;
		GaclManager::gaclAcl = GRSTgaclAclNew ();
	}
	if ( gaclAcl == NULL ){
		throw GaclException (__FILE__, __LINE__,  "newCredential (WMPgaclCredType,string, string, string)" , -1,
			"unable to create new gacl" );
	}

	// new entry
	gaclEntry = GRSTgaclEntryNew ( );
	if ( GaclManager::gaclEntry == NULL ){
		throw GaclException (__FILE__, __LINE__,  "newCredential (WMPgaclCredType, string, string, string)" , -1,
			"fatal error; unable to create new entry" );
	}

	gaclCred = GRSTgaclCredNew ((char *)credType.c_str());

	if ( credType.compare(WMPGACL_ANYUSER_CRED) != 0 )
	 	GRSTgaclCredAddValue(gaclCred, (char*)rawName.c_str() , (char*)rawValue.c_str());

	if ( GaclManager::gaclCred == NULL ){
		throw GaclException (__FILE__, __LINE__,  "newCredential (string, string, string)" , -1,
			"unable to create new credential" );
	}

	// new user
	gaclUser  = GRSTgaclUserNew (gaclCred) ;

	if ( GaclManager::gaclUser == NULL ){
		throw GaclException (__FILE__, __LINE__,  "newCredential (string, string, string)" , -1,
			"fatal error; unable to create new user credential ");
	}


	// adds the created credential to the entry
	GRSTgaclEntryAddCred ( GaclManager::gaclEntry,  GaclManager::gaclCred);

	// adds the entry to the gacl
	GRSTgaclAclAddEntry (GaclManager::gaclAcl, GaclManager::gaclEntry);
};

int GaclManager::loadNewCredential ( string file, WMPgaclCredType type, string dn, bool unset_perm, bool create) {

	switch ( type ){
		case WMPGACL_ANYUSER_TYPE : {
				GaclManager::credType =WMPGACL_ANYUSER_CRED ;
				GaclManager::rawName = "" ;
				GaclManager::rawValue = "" ;
				break;
		}
		case WMPGACL_PERSON_TYPE: {
				GaclManager::credType = WMPGACL_PERSON_CRED ;
				GaclManager::rawName = WMPGACL_PERSON_TAG ;
				GaclManager::rawValue = dn ;
				break;
		}
		case  WMPGACL_DNLIST_TYPE: {
				GaclManager::credType = WMPGACL_PERSON_CRED ;
				GaclManager::rawName = WMPGACL_PERSON_TAG ;
				GaclManager::rawValue = dn ;
				break;
		}
		default : {
			throw GaclException (__FILE__, __LINE__,  "newCredential (string, WMPgaclCredType,string, bool)" , -1,
				"wrong credential type :" + type );
			}
	}

	GaclManager::gaclFile = file;

	edglog(debug)<<"checking gacl file ...."<< endl;

	if ( ! create ) {
		if ( gaclExists ( file ) ){
			edglog(debug)<<"loading from file....."<< endl;
			GaclManager::loadFromFile (file);
			edglog(debug)<<"loading Credential ...."<< endl;
			GaclManager::loadCredential (unset_perm) ;
		} else {
			newGacl ( );
			newCredential ( ) ;
		}
	} else {

		newGacl ( );
		newCredential ( ) ;
	}

};
/*
 * loads the credential for the user
 * @user user name
 * @param rawname
 * @param rawvalue
*/

int GaclManager::loadCredential ( bool unset_perm ) {

	GRSTgaclCred  *cred = NULL;
	GRSTgaclEntry *entry = NULL;
	GRSTgaclUser *user = NULL;

	bool found = false;
	bool raw = false;

	GRSTgaclNamevalue *nv = NULL;
	char * name = NULL;
	char *value = NULL;

	edglog(debug)<<"loadCredential-user =" << user<< endl;

	if (gaclAcl !=NULL) {

		if ( rawName.size()>0 && rawValue.size()>0 ){
			raw = true;
		}

		entry = gaclAcl-> firstentry ;

		while  (entry != NULL ) {
			edglog(debug)<<"entry not null" << user<< endl;
			cred = entry->firstcred ;

			while ( cred != NULL ) {

				edglog(debug)<< "cred-type:" << cred->type<< endl;

				if ( strcmp( cred->type, (char*)credType.c_str()) == 0 ){

					edglog(debug)<< "cred ok" <<endl;
					//char *dnlist = cred->dnlist ;

					if (raw) {
						nv = cred->firstname;

						if (nv) {
							name = nv->name;
							value = nv->value ;

							if (name && value){

								edglog(debug) << "name=" << name << "\n";
								edglog(debug) << "value=" << value << "\n";

								if ( strcmp(name,(char*)rawName.c_str()) == 0 &&
									strcmp(value,(char*)rawValue.c_str() ) ==0 ){
										found = true;
										edglog(debug) << "found !!!!" << "\n";
										break;
								}
							}
						}
					} else{

						found = true;
						break;
					}
				}
				cred = (GRSTgaclCred*) cred->next;
			} // while(cred)

			if ( !found ) {
				entry = (GRSTgaclEntry*) entry->next;
			} else {
				break;
			}

		} //while(entry)

	} //if


	if (entry != NULL ) {

		if (unset_perm){
				entry->allowed = GRST_PERM_NONE;
				entry->denied = GRST_PERM_NONE;
		}

		GaclManager::gaclEntry = entry ;
		GaclManager::gaclCred = cred ;
		GaclManager::gaclUser  = GRSTgaclUserNew (cred);

		GaclManager::gaclAllowed = entry->allowed ;
		GaclManager::gaclDenied = entry->denied;

		GaclManager::gaclPerm = GRSTgaclAclTestUser (GaclManager::gaclAcl, GaclManager::gaclUser);

	} else {
		GaclManager::newCredential ( );
	}

	if (found)
		return WMPGACL_SUCCESS ;
	else
		return WMPGACL_ERROR ;



};



