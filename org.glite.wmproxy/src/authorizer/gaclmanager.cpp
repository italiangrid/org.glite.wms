
#include "gaclmanager.h"

using namespace std;

using namespace glite::wmsutils::exception; //Exception


char* perm_vect[ ] ={
GACLMGR_PERM_NONE	,
GACLMGR_PERM_READ	,
GACLMGR_PERM_EXEC	,
GACLMGR_PERM_LIST	,
GACLMGR_PERM_WRITE	,
GACLMGR_PERM_ADMIN
};


/*
*	Default constructor
*/
GaclManager::GaclManager ( ) {

	 GRSTgaclInit();

	GaclManager::gaclAcl = GRSTgaclAclNew () ;
	GaclManager::gaclEntry =  GRSTgaclEntryNew () ;
	GaclManager::gaclCred = NULL ;
	GaclManager::gaclAllowed = GRST_PERM_NONE  ;
	GaclManager::gaclDenied = GRST_PERM_NONE ;

}

/*
 * Constructor.
 * @param file path location of the gacl file
 * @param create true = create the file if it doesn't exist (default value)
 */

GaclManager::GaclManager (std::string file, bool create) {

	gaclAcl = NULL ;
	gaclCred = NULL ;
	gaclUser = NULL ;
	GaclManager::gaclEntry =  GRSTgaclEntryNew () ;
	GaclManager::gaclAllowed = GRST_PERM_NONE ;
	GaclManager::gaclDenied = GRST_PERM_NONE ;

	GRSTgaclInit();


	GaclManager::gaclFile = file;

	if ( gaclExists ( file )){
		//cout << "GACL exists !!" << endl;
		GaclManager::loadFromFile (file);
	} else{
		cout << "exception !!" << endl;
		if ( !create)
			throw GaclException (__FILE__, __LINE__,  "GaclManager(string, bool)" , -1, "gacl file not found: (" + file +")" );
	}

}
/*
 * Gets the code number of the permission
 * @param permission permission string (GACLMGR_PERM_xxxxx)
 */

int GaclManager::getPermissionCode (std::string permission){
	int code = -1;

	for (int i=0; i < GACLMGR_NUMDEF_CODES; i++){
		if ( strcmp (perm_vect[i] , (char*)permission.c_str() ) == 0 ){
			code = i;
			break;
		}
	}

	return code;
}
/*
	CHECK-METHODS

*/
/*
 * Checks if the permission is allowed to user
 * @user username
 * @param permission permission string (GACLMGR_PERM_xxxxx)
 * @return 1 if the permission is allowed ; 0 is not allowed ;
 *               -1 in case of bad gacl format (both allow and deny are set)
 *		-2 in case of bad gacl format (neither allow or deny is set)
 */
int GaclManager::checkAllowPermission (std::string user, std::string permission) {

	bool allow, deny = false;
	GRSTgaclPerm perm ;
	int result = -1;
	std::cout << "checkAllowPermission - user =  " << user  << std::endl;
	std::cout << "checkAllowPermission - permission =  " <<permission<< std::endl;


	GaclManager::loadUserCredential ( user );

	perm = GRSTgaclPermFromChar( (char*) permission.c_str() );

	// TBR
	//printf("perm  = %d [%s]\n", perm, GRSTgaclPermToChar(perm) );

	allow = ! (! (GaclManager::gaclAllowed & perm)  );
	cout << "allow = " << allow << endl;
	deny = ! (! (GaclManager::gaclDenied & perm)  );
	cout << "deny = " <<deny<< endl;

	if ( allow && deny ) {
		cout << "Exception: both allow and deny are set!!!" << endl;
		result -1 ;
	 } else  {
	 	if ( !allow && !deny ) {
	 		cout << "Exception: neither allow or deny is  set!!!" << endl;
			result = -2;
		 } else {
			result = (int) allow ;
		 }
	 }
	 cout << "GaclManager::checkAllowPermission> result=" <<result << endl;
	//allow = GRSTgaclPermHasExec(gaclPerm);
	return result ;

 };

/*
 * Checks if the read permission is allowed to anyuser
 */

int GaclManager::checkAllowReadForAnyUser ( ){

	return checkAllowPermission(GACLMGR_ANYUSER, GACLMGR_PERM_READ);

 };
/*
 * Checks if the write permission is allowed to anyuser
 */
int GaclManager::checkAllowWriteForAnyUser ( ){

	return checkAllowPermission(GACLMGR_ANYUSER, GACLMGR_PERM_WRITE);

 };

/*
 * Checks if the list permission is allowed to anyuser
*/
int GaclManager::checkAllowListForAnyUser ( ){

	return checkAllowPermission(GACLMGR_ANYUSER, GACLMGR_PERM_LIST);

 };

/*
 * Checks if the exec permission is allowed to anyuser
*/
int GaclManager::checkAllowExecForAnyUser ( ){

	return checkAllowPermission(GACLMGR_ANYUSER, GACLMGR_PERM_EXEC);

 };


/*
	addPermission-METHODS

*/
/*
 * allows permission to the user
 * @user user name
 * @param permission permission string (GACLMGR_PERM_xxxxx)
 * @param rawname
 * @param rawvalue
*/
 int GaclManager::allowPermission(std::string user, std::string permission, std::string rawname,  std::string rawvalue) {

	GRSTgaclPerm perm = 0;
	int result = -1 ;
	std::cout << "allowPermission - user =  " << user  << std::endl;
	std::cout << "allowPermission - rawname =  " << rawname  << std::endl;
	std::cout << "allowPermission - rawvalue=  " << rawvalue  << std::endl;
	if ( gaclAcl != NULL ) {
		GaclManager::loadUserCredential ( user, rawname, rawvalue );
	} else {
		GaclManager::newUserCredential ( user, rawname, rawvalue );
	}
	/*
	else {
		GaclManager::newUserCredential ( user, rawname, rawvalue );
	}
	*/

	perm = GRSTgaclPermFromChar ( (char *) permission.c_str( ) ) ;

	GRSTgaclEntryAllowPerm (GaclManager::gaclEntry, perm) ;

	GRSTgaclEntryUndenyPerm (GaclManager::gaclEntry, perm) ;

	result = GaclManager::saveGacl ( GaclManager::gaclFile );

	return result ;

 };

/*
 * denies permission to the user
 * @user user name
 * @param permission permission string (GACLMGR_PERM_xxxxx)
 * @param rawname
 * @param rawvalue
*/
 int GaclManager::denyPermission(std::string user, std::string permission, std::string rawname,  std::string rawvalue ) {

	GRSTgaclPerm perm = 0;
	int result = -1 ;
	std::cout << "allowPermission - user =  " << user  << std::endl;
	std::cout << "allowPermission - rawname =  " << rawname  << std::endl;
	std::cout << "allowPermission - rawvalue=  " << rawvalue  << std::endl;
	if ( gaclAcl != NULL ) {
		GaclManager::loadUserCredential ( user, rawname, rawvalue );
	} else {
		//cout << "gacl is null .... new user ..." << endl ;
		GaclManager::newUserCredential ( user, rawname, rawvalue );
	}

	perm = GRSTgaclPermFromChar ( (char *) permission.c_str( ) ) ;

	//cout << "perm = "  << perm << endl;

	GRSTgaclEntryDenyPerm (GaclManager::gaclEntry, perm) ;
	GRSTgaclEntryUnallowPerm (GaclManager::gaclEntry, perm) ;

	result = GaclManager::saveGacl ( GaclManager::gaclFile );

	return result ;
 };

/*
 * allows the read permission to anyuser
*/
int GaclManager::allowReadToAnyUser  (  ) {


	return GaclManager::allowPermission ( GACLMGR_ANYUSER, GACLMGR_PERM_READ);

};
/*
 * allows the write permission to anyuser
*/
int GaclManager::allowWriteToAnyUser  (  ) {

	return GaclManager::allowPermission ( GACLMGR_ANYUSER, GACLMGR_PERM_WRITE);

};
/*
 * allows the exec permission to anyuser
*/
int GaclManager::allowExecToAnyUser  (  ) {

	return GaclManager::allowPermission ( GACLMGR_ANYUSER, GACLMGR_PERM_EXEC);
};

/*
 * allows the list permission to anyuser
*/
int GaclManager::allowListToAnyUser  (  ) {

	return GaclManager::allowPermission ( GACLMGR_ANYUSER, GACLMGR_PERM_LIST);
};

/*
 * denies the read permission to anyuser
*/
int GaclManager::denyReadToAnyUser  (  ) {

	return GaclManager::denyPermission ( GACLMGR_ANYUSER, GACLMGR_PERM_READ);
};

/*
 * denies the write permission to anyuser
*/
int GaclManager::denyWriteToAnyUser  (  ) {

	return GaclManager::denyPermission ( GACLMGR_ANYUSER, GACLMGR_PERM_WRITE);
};

/*
 * denies the exec permission to anyuser
*/
int GaclManager::denyExecToAnyUser  (  ) {

	return GaclManager::denyPermission ( GACLMGR_ANYUSER, GACLMGR_PERM_EXEC);
};

/*
 * denies the list permission to anyuser
*/
int GaclManager::denyListToAnyUser  (  ) {

	return GaclManager::denyPermission ( GACLMGR_ANYUSER, GACLMGR_PERM_LIST);
};

/*
 * saves the gacl to the file
 * @param file location where to save the file
*/
int GaclManager::saveGacl ( std::string file ){

	int result = GRSTgaclAclSave (GaclManager::gaclAcl, (char*)file.c_str() );

	if ( result == 0)
		return GACLMGR_ERROR;
	else
		return GACLMGR_SUCCESS;
};

/*
 * loads the gacl to the file
 * @param file the location of the file
*/
void GaclManager::loadFromFile( std::string file ) {

	GaclManager::gaclAcl = GRSTgaclAclLoadFile ( (char*) file.c_str() );

	if ( gaclAcl == NULL ) {

		cout << "bad gacl format in file: " + file << endl ;
		exit (-1);
		//throw GaclException (__FILE__, __LINE__,  "GaclManager::loadFromFile" , -1, "bad gacl format in file: " + file );
	}

};

/*
 * prints the gacl
*/
void GaclManager::printGacl ( ) {

	if ( GaclManager::gaclAcl != NULL){
		GRSTgaclAclPrint (GaclManager::gaclAcl, stdout );
	} else
		std::cout << "no gacl loaded" << std::endl;

};

/*
 * checks whether the gacl file exists
 * @param file the location of the gacl file
*/
bool GaclManager::gaclExists (std::string file) {
    FILE* fptr = NULL;
    bool exist = false;

    fptr = fopen ( (char*) file.c_str() , "r");
    if (fptr != NULL) {
    	exist = true;
	fclose (fptr);
    }

    return exist;
}
/*
 * creates the credential for the user
 * @user username the user
 * @param rawname
 * @param rawvalue
*/
void GaclManager::newUserCredential ( std::string username, std::string rawname,  std::string rawvalue ){

	int result = - 1;
	std::string errmsg = "";

	// new acl
	if ( GaclManager::gaclAcl  == NULL ){
		GRSTgaclInit ( ) ;
		GaclManager::gaclAcl = GRSTgaclAclNew ();
	}
	if ( GaclManager::gaclAcl == NULL ){
		errmsg = "fatal error; unable to create new gacl for: " +username;
		throw GaclException (__FILE__, __LINE__,  "newUserCredential (string, string, string)" , -1, errmsg );
	}

	// new entry
	GaclManager::gaclEntry = GRSTgaclEntryNew ( );
	if ( GaclManager::gaclEntry == NULL ){
		errmsg = "fatal error; unable to create new entry for: " +username;
		throw GaclException (__FILE__, __LINE__,  "newUserCredential (string, string, string)" , -1, errmsg );
	}

	// new credential
	GaclManager::gaclCred = GRSTgaclCredNew ((char *)username.c_str());

	if ( GaclManager::gaclCred == NULL ){
		errmsg = "fatal error; unable to create new credential for: " + username;
		throw GaclException (__FILE__, __LINE__,  "newUserCredential (string, string, string)" , -1, errmsg);
	}

	// new user
	gaclUser  = GRSTgaclUserNew ( GaclManager::gaclCred) ;

	if ( GaclManager::gaclUser == NULL ){
		errmsg = "fatal error; unable to create new user credential for: " + username ;
		throw GaclException (__FILE__, __LINE__,  "newUserCredential (string, string, string)" , -1, errmsg);
	}

	if ( rawname.size( ) > 0 && rawvalue.size( ) > 0 ) {
	cout << "adding raw !!" << endl;
		result = GRSTgaclCredAddValue( GaclManager::gaclCred, (char*)rawname.c_str(), (char*)rawvalue.c_str());
	} else
	cout << "raw no !!" << endl;

	// adds the created credential to the entry
	GRSTgaclEntryAddCred ( GaclManager::gaclEntry,  GaclManager::gaclCred);

	// adds the entry to the gacl
	GRSTgaclAclAddEntry (GaclManager::gaclAcl ,GaclManager::gaclEntry);

};
/*
 * loads the credential for the user
 * @user user name
 * @param rawname
 * @param rawvalue
*/

int GaclManager::loadUserCredential ( std::string username, std::string rawname,  std::string rawvalue  ) {

	GRSTgaclCred  *cred = NULL;
	GRSTgaclEntry *entry = NULL;
	GRSTgaclUser *user = NULL;

	bool found = false;
	bool raw = false;

	GRSTgaclNamevalue *nv = NULL;
	char * name = NULL;
	char *value = NULL;

	//std::cout << " loadUserCredential-user =" << username << std::endl;

	if (gaclAcl !=NULL) {

		if ( rawname.size()>0 && rawvalue.size()>0 ){
			raw = true;
		}
		entry = GaclManager::gaclAcl-> firstentry ;
		/*
		if ( entry !=  NULL )
			cout << "entry is not null" << std::endl ;
		else
			cout << "entry: NULL" << std::endl ;
		*/
		//std::cout << " loadUserCredential-2" << std::endl;
		while  (entry != NULL ) {

			cred = entry->firstcred ;

			while ( cred != NULL ) {

				//cout << "cred-type:" << cred->type << std::endl ;
				/*
				int has = GRSTgaclUserHasCred (gaclUser, cred);
				printf ("has=%d\n", has);
				*/
				if ( strcmp( cred->type, (char*)username.c_str()) == 0 ){
					//cout << "credential ok !!!" << std::endl ;

					//char *dnlist = cred->dnlist ;

					if (raw) {
						nv = cred->firstname;

						if (nv) {
							name = nv->name;
							value = nv->value ;

							if (name && value){

								cout << "name=" << name << "\n";
								cout << "value=" << value << "\n";

								if ( strcmp(name,(char*)rawname.c_str()) == 0 &&
									strcmp(value,(char*)rawvalue.c_str() ) ==0 ){

										found = true;
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
		GaclManager::gaclEntry = entry ;
		GaclManager::gaclCred = cred ;
		GaclManager::gaclUser  = GRSTgaclUserNew (cred);
		GaclManager::gaclAllowed = entry->allowed ;
		GaclManager::gaclDenied = entry->denied;
cout <<"gaclAllowed =" << GaclManager::gaclAllowed << endl;
cout <<"gaclDenied=" << GaclManager::gaclDenied << endl;
		GaclManager::gaclPerm = GRSTgaclAclTestUser (GaclManager::gaclAcl, GaclManager::gaclUser);

	} else {
		GaclManager::newUserCredential ( username, rawname,rawvalue);
	}

	if (found)
		return GACLMGR_SUCCESS ;
	else
		return GACLMGR_ERROR ;



};


