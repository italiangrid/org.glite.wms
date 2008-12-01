/*| 
 * This is a little ldap test program
 * 
 * author Geerd-Dietger.Hoffmann@cern.ch
 * massively changed by Felix.Ehm@cern.ch
 * 
 * This is the h file for ldapbench.c
 */

/*| Actually goes of to the ldap server and does the query */
int doTheQuery();

/*| Prints a little box with the progress */
void printProgress();

/*| Is the function that is invoced by the thread */
void * myLittleThread(void *threadid);

/* Measures the time to execute a function*/
double measureExecutionTime(int (*function)());

/*| Just a little function to copy a segment of a string into a string */
char * strpst (char *resultString, char *source, char *topaste, char chartofind);

/* Structure to keep data from each thread. */
struct thread_data {
	unsigned int id;
	double avg_time;
	unsigned long int connections;
};


