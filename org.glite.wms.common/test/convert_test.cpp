#include <iostream>
#include <fstream>

#include <classad_distribution.h>

#include "../src/common_namespace.h"
#include "../src/requestad/convert.h"
using namespace std ;
USING_COMMON_NAMESPACE;

const string   basead( "[ "
		       "  Executable = \"/bin/echo\";"
  		       "  Universe = \"Globus\";"
  		       "  Transfer_Executable = true;"
  		       "  Copy_to_Spool = true;"
  		       "  Arguments = \"Ciao\";"
  		       "  Output = \"/var/tmp/output\";"
		       "  Type = \"job\";"
		       //  		       "  Error = \"/var/tmp/error\";"
  		       "  GlobusScheduler = \"lxde01.pd.infn.it:2119/jobmanager-pbs\";"
  		       "  x509userproxy = \"/tmp/x509up_u723\";"
  		       "  UserSubjectName=\"/C=IT/O=INFN/OU=Personal "
  		       "Certificate/L=Padova/CN=Rosario Peluso/Email=Rosario.Peluso@pd.infn.it\";"
  		       "  dg_jobId = \"Condor_test_0\";"
  		       "  GlobusRSL = \"(queue=pbsque)\";"
  		       "  Log = \"/var/tmp/CondorG.log\";"
		       "];" );

int main( void )
{
  ofstream                 ofs( "/var/tmp/submit.test" );
  classad::ClassAd        *ad;
  classad::ClassAdParser   parser;

  cout << basead << endl;

  ad = parser.ParseClassAd( basead.c_str() );

  if( ad != NULL ) {
    ad->Puke();

    requestad::to_submit_stream( ofs, *ad );
  }
  else cerr << "Parsing error... " << endl;

  ofs.close();

  return 0;
}
