/* ************************************************
    JobId Example Method
**************************************************/

#include "../src/utilities/Exceptions.h"
#include "../src/jobid/JobId.h"

using namespace edg::workload::common::utilities ;
using namespace edg::workload::common::jobid ;
using namespace std ;

int main(int argc,char *argv[]){
   if (argc < 3 ||  strcmp(argv[1],"--help") == 0)  {
     cout << "This Example will generate several JobId starting from a give lb address:\n"
     cout << "Usage : " << argv[0] << "<lbHost> <lbPort> [ <number of JobId to be generated>]" << endl;
     return 1;
   }
 
   try{
     int n = argc=4? argv[3] : 1 ;
     for (int i = 0 ; i<n ; i++ ) {
        JobId jobid;
        jobid.setJobId ( argv[1] , argv[2] ) ;
        cout <<" - " << jobid.toString() ;
     }
   } catch (Exception &exc) {
	cout << "\n\nException found!!!.\nHere follows the error description:" ;
	cout  <<  "\nStack Trace:\n"  ;
	cout << exc.printStackTrace() ;
	cout  << "\nWhat:\n"  ;
	cout << exc.what() ;
   } catch (exception &exc ){
	cout  << "\nstd exception found : What:\n"  ;
	cout << exc.what() ;
   } catch (...) {
	cout  << "\n\n\n WARNING!!!!!\nUnkwnow exception, please contact the developer\n"  ;
   }

   cout << "\n" ;
   return 0;
};

