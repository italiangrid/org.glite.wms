/* LDAP server stress test
 *
 * This is a little ldap test program which allows stress testing 
 * of an openldap server.
 * 
 * author Geerd-Dietger.Hoffmann@cern.ch
 * massively changed by Felix.Ehm@cern.ch
 *
 */

// Some standard stuff
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

//For the timing
#include <sys/time.h>

//The ldap stuff
#include <ldap.h>

//The thread stuff
#include <pthread.h>

/* And now the h file */
#include "ldapbench.h"

/* The DB file */
#include "dbase.h"

/*-----------------------------------EDIT---------------------------------------------*/
/* Here some configurations 
   Please edit before compile
*/
/* Debug output on(1) or off(0)*/
int     debug              = 0;

/* Do a scan of the results return through the ldap server. This is client side ! */
int     doscan             = 0;

/* The host where the ldap server resides */
char    *ldapHost          = NULL;

/* The port to connect to */
int     ldapPort           = 389;

/* Your login */
char    *loginDN           = "cn=Manager,dc=example,dc=edu";

/* And the password */
char    *password          = "letmein";

/* Where to search */
char    *searchBase        = "ou=People,dc=example,dc=edu";    

/* What to search for : for everything specify (objectClass=*) */
char    *objClass          = "(&(objectClass=posixAccount)(uid=ribalba))";

/* How many times a thread should run */
int     executions         = 10;

/* How many threads there should be */
int     num_pthreads       = 5;

/* The timeout as timeval struct. Default is unlimited */
struct timeval timeout;


#define DEFAULT_RUNTIME 10
/* The duration the test should run for */
int runTime       = DEFAULT_RUNTIME;

/*------------------------------END--EDIT---------------------------------------------*/

/* "\n Usage:   search <host name> <port number> <login dn> <password> <search base> [<object class>] \n" */
/* static char usage[] =""; */
void usage(){
	printf("\n");
	printf(" Usage:   ldapbench -H <hostname>\n");
	printf("\n");
	printf("option | explanation                               | default value\n");
	printf("------------------------------------------------------------------\n");
	printf(" -H <host name>    | The target host               |\n");
	printf(" -p <port>         | The port of target            | %d\n", ldapPort );
	printf(" -D <login dn>     | Login dn                      | %s\n", loginDN );
	printf(" -w <password>     | Login passord                 | %s\n", password );
	printf(" -b <base>         | Search base                   | %s\n", searchBase );
	printf(" -x                | No authorisation              | off \n");
	printf("                   | (disables -p/-D)              |\n");
  	printf(" -l <seconds>      | Timeout in [sec]              | %d \n", (int)timeout.tv_sec);
	printf(" -f <objectclass>  | ObjectClass to search         |\n");
        printf(" -d <query_file    | File containing ldap filters  |\n");
        printf("                   | (disables -f)                 |\n");
	printf(" -e <loops>        | loops/thread                  | %d\n", executions );
	printf(" -t <threads>      | Number of threads to use      | %d\n", num_pthreads );
	printf(" -v                | Verbose mode                  | off\n");
	printf(" -s                | Scan resultset (slow)         | off\n");
	printf("\n" );
	printf(" Example: ./ldapbench -H \"lxb5479\" -p \"389\" -D \"[login]\" -w \"[passwd]\" -b \"dc=example,dc=edu\" -f \"(objectclass=*)\"\n" );
	printf("\n");
}

/* Some error gathering */
int e_sessionInit = 0;
int e_logonfail = 0;
int e_searchfail = 0;
int e_searchtimeout=0;
int e_searchnoresults=0;

/* The average ldapsearch time for each thread 
   the array is allocated in main()
*/
struct thread_data **t_data = NULL;
int version=LDAP_VERSION3; /* So we don't have to mess around mith version 2 */


/* pointer to our DB file */
dbase *dbFile = NULL;


/**************************************
 * MAIN starts here 
 *
 **************************************/
int main( int argc, char **argv )
{   

  int         i = 0;
  pthread_attr_t attr;
  void *status;
  int         c = 0;
  extern char *optarg;
  extern int  optind, optopt, opterr;
  char *      query_file = NULL;

  /* The default timeout */
  timeout.tv_sec = 20;
 
  /* A nice rewrite with getopt (I know some people don't like it but I use it)*/
  while ((c = getopt(argc, argv, "H:p:vsD:w:b:f:l:t:hxe:r:d:")) != -1) {
    switch(c) {
    case 'H':
      ldapHost = optarg;
      if( debug )
	printf("\tldapHost is %s\n",ldapHost );
      break;
    case 'p':
      ldapPort = atoi ( optarg ) ;
      if( debug )
	printf("\tldapPort is %d\n",ldapPort );
      break;
    case 'v':
      printf("\tDebug or Verbose is enabled\n");
      debug = 1;
      break;
    case 's':
      if( debug )
	printf("\tThe search results are scanned on the client side. SLOW\n");
      doscan = 1;
      break;
    case 'D':
      loginDN = optarg;
      if ( debug )
	printf("\tloginDN is %s\n", loginDN);
      break;
    case 'w':
      password = optarg;
      if ( debug )
	printf("\tpassword is %s\n", password);
      break;
    case 'b':
      searchBase = optarg;
      if ( debug )
	printf("\tsearchBase is %s\n", searchBase);
      break;
    case 'f':
      objClass = optarg;
      if ( debug )
	printf("\tobjClass is %s\n", objClass);
      break;
    case 'e':
      executions = atoi ( optarg );
      if ( debug )
	printf("\texecutions is %d\n", executions);
      break;
    case 'l':
      timeout.tv_sec = atoi ( optarg );
      if ( debug )
	printf("\tTimeout is %d\n", (int)timeout.tv_sec);
      break;
    case 't':
      num_pthreads = atoi ( optarg );
      if ( debug )
	printf("\tnum_pthreads is %d\n", num_pthreads);
      break;
    case 'r':
      runTime = atoi ( optarg );
      if ( debug )
	printf("\trunTime is %d\n", runTime);
      break;
    case 'd':
      objClass = NULL;
      query_file = optarg;
      if ( debug )
	printf("\tDB file is %s\n", optarg);
      break;
    case 'x':
      loginDN =  NULL;
      password = NULL;
      if ( debug )
	printf("\tAuthentication disabled\n");
      break;

    case 'h':
    case '?':
      usage();
      return ( 1 );
      break;
    }
  }
  
  if ( !ldapHost ) {
    fprintf (stderr, "No host given!\n");
    usage();
    exit (1);
  }
  
  if ( query_file ) {
        dbFile = init( query_file, debug );
  }
  
  
  printf("# Start ... \n");
  
    
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_t thread[num_pthreads];
  t_data = (struct thread_data**)malloc(sizeof(struct thread_data*) * num_pthreads);
  

  for (i = 0; i < num_pthreads; i++) {
    
    t_data[i] = (struct thread_data*)malloc(sizeof(struct thread_data));
    
    if (pthread_create(&thread[i], &attr, &myLittleThread , (void *)(long)i)) {
      fprintf(stderr, "Error creating a new thread \n");
      exit(1);
    }
  }

  /* Free attribute and wait for the other threads */
  pthread_attr_destroy(&attr);

  for(i=0; i<num_pthreads; i++) {
      pthread_join(thread[i], &status);
  }
  
  /* Print the result and free the avg_times */
  printProgress();
  for (i = 0; i < num_pthreads; i++) {
  	free( t_data[i] );
  }
  free ( t_data );
  pthread_exit(NULL);

  return( 0 );
}





/**************************************
 * Worker function for thread.
 * Executes a query a given number of times
 *
 **************************************/
void * myLittleThread(void *threadid){
  int my_id = (int)(long)threadid;
  int i = 0;
  double total = 0 ;
  struct timeval startTime, currentTime;
  t_data[my_id]->id = my_id;
  t_data[my_id]->avg_time = 0;
  t_data[my_id]->connections = 0;


  /* do loop execution */
  if ( runTime == DEFAULT_RUNTIME ) {
    /* Do the query in a loop */
    if ( debug ) printf( "# Thread %d: starting %d executions\n", my_id, executions );
    for(i=0; i < executions; i++){
      total += measureExecutionTime( &doTheQuery );
      t_data[my_id]->connections++;
    }
  }
  
  /* Repeat the query for a given time */
  else {
    if ( debug )printf( "# Thread %d: executing for %d seconds\n", my_id, runTime );
    gettimeofday(&startTime,0);
    gettimeofday(&currentTime,0);
    while ( currentTime.tv_sec < startTime.tv_sec+runTime ) {
      total += measureExecutionTime( &doTheQuery );
      gettimeofday( &currentTime, 0 );
      t_data[my_id]->connections++;
    }
  }

  /* Tell the main() what our average is */
  t_data[my_id]->avg_time = total/t_data[my_id]->connections;
  
  fflush(stdout);
  return ( NULL );
}






/**************************************
 * Measures the time the function given as an argument
 * takes (wrapper for doTheQuery)
 *
 * return : the time of execution 
 **************************************/
double measureExecutionTime(int (*function)()){
  /* For time measuring */
  struct timeval   tv0;
  double           t1=0, t2=0;
  int              ret = 0;
  
  gettimeofday(&tv0,0);
  t1=tv0.tv_sec+(tv0.tv_usec/1000000.0);
  
  ret = (*function)();
  
  if ( ret ) {  
    gettimeofday(&tv0,0);
    t2 = tv0.tv_sec + (tv0.tv_usec/1000000.0);
    printf("%.6lf\t%.6lf\t%.6lf\n", t1, t2, t2-t1);
    return t2-t1;
  }

  return 0;
}





/**************************************
 * Opens ldap connection and performs a search
 *
 **************************************/
int doTheQuery(){

  int          i=0, rc=0, entryCount=0;
  char        *attribute=NULL, *dn=NULL, **values;
  BerElement  *ber=NULL;
  LDAP        *ld=NULL;
  LDAPMessage *searchResult=NULL, *entry=NULL;
  char* objClassPointer;
  char *resultString=NULL;

  /* Initialize the LDAP session */
  ld = ldap_init( ldapHost, ldapPort );
  if (ld == NULL){
    fprintf (stderr,"\n    LDAP session initialization failed\n");
    e_sessionInit++;
    return 0;
  }

  /* Set LDAP version to 3 and set connection timeout(In sec).*/
  ldap_set_option( ld, LDAP_OPT_PROTOCOL_VERSION, &version);
  /*ldap_set_option( ld, LDAP_OPT_TIMELIMIT, &timeout.tv_sec); */


  if(debug)
    printf ( "# LDAP session initialized\n");


  /* Bind to the server */
  rc = ldap_simple_bind_s( ld, loginDN, password );
  
  if (rc != LDAP_SUCCESS ){
    fprintf (stderr, "ldap_simple_bind_s: %s\n", ldap_err2string( rc ));
    e_logonfail++;
    ldap_unbind_s ( ld );
    return 0;
  }


  if(debug)
    printf("# Bind successful\n");

  if ( dbFile ){
    objClassPointer = get_random_entry( dbFile );
  }
  else{
    objClassPointer = objClass;
  }
  
  if (debug ) printf("# Searching with filter %s\n",objClassPointer);


  /* Search the directory */
  rc =  ldap_search_ext_s(
                          ld, searchBase, LDAP_SCOPE_SUBTREE, 
                          objClassPointer, NULL, 0, NULL, NULL, 
                          &timeout, LDAP_NO_LIMIT, &searchResult
                          );


  /* Error handling */
  if ( rc != LDAP_SUCCESS ){
    if  ( rc == LDAP_TIMEOUT ) {
    	fprintf (stderr, "TIMEOUT\n");
	e_searchtimeout++;
    }
    else if ( rc == LDAP_NO_RESULTS_RETURNED ) {
        fprintf (stderr, "NO RESULTS\n");
	e_searchnoresults++;
    }
    else {
	fprintf (stderr, "ldap_search_ext_s: %s\n", ldap_err2string( rc ));
	e_searchfail++;

    }
    ldap_msgfree( searchResult );
    ldap_unbind_s( ld );
    return 0;
  }   

  /* Check, if we got sth. back */     
  entry = ldap_first_entry( ld, searchResult );
  entryCount = ldap_count_entries( ld,searchResult );
  
  
  if ( ! entryCount ) {
  	fprintf (stderr, "No Results\n");
	e_searchnoresults++;
  }
  

  /* To save client CPU time in test 10% */
  if(doscan){
    /* Go through the search results by checking entries */
    for (   entry   =   ldap_first_entry( ld, searchResult ); 
	    entry   !=  NULL; 
	    entry   =   ldap_next_entry( ld, entry ) ) 
      {
	if (( dn = ldap_get_dn( ld, entry )) != NULL ){

	  if(debug)
	    printf("\n   dn: %s\n", dn );
	  ldap_memfree( dn );
	}
            
	for (   attribute = ldap_first_attribute( ld, entry, &ber );
		attribute != NULL; 
		attribute = ldap_next_attribute( ld, entry, ber ) ) 
	  {   
	    /* Get values and print.  Assumes all values are strings. */
	    if (( values = ldap_get_values( ld, entry, attribute)) != NULL ){
	      for ( i = 0; values[i] != NULL; i++ ){

		if(debug)	 
		  printf("        %s: %s\n", attribute, values[i] );

	      }
	      ldap_value_free( values );
	    }
	    ldap_memfree( attribute );
	  }
	ber_free(ber, 0);
      }
    entryCount = ldap_count_entries( ld, searchResult );

    if(debug)
      printf("Search completed successfully.\n    Entries  returned: %d\n", entryCount);
  }
 
  if ( searchResult ) ldap_msgfree( searchResult );
  if ( resultString ) free(resultString);    

  ldap_unbind_s( ld );

  // For speed
  fflush(stdout);
  

  return 1;
}



/***************************************
 * Prints the result 
 *
 **************************************/
void printProgress(){
  double avg=0;
  unsigned long int conn=0;
  int i;
  for (i=0; i<num_pthreads; i++ ) {
     avg  += t_data[i]->avg_time;
     conn += t_data[i]->connections;
     printf("# Thread %d: %.6lf %ld\n", t_data[i]->id ,
                                        t_data[i]->avg_time,
					t_data[i]->connections);
  }

  printf("# --------------SUM--------------\n");
  printf("# e_sessionInit    : %d\n", e_sessionInit );
  printf("# e_logonfail      : %d\n", e_logonfail );
  printf("# e_searchfail     : %d\n", e_searchfail  );
  printf("# e_searchnoresults: %d\n", e_searchnoresults  );
  printf("# e_searchtimeout  : %d\n", e_searchtimeout  );
  printf("# avg_searchTime   : %.6lf\n", avg/conn  );
  printf("# connections      : %ld\n", conn  );
  printf("# -------------------------------\n");
}
