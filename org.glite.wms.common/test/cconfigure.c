#include <stdio.h>

#include "../src/configuration/cwrapper.h"

int main( void )
{
  int                            ret = 0, val;
  size_t                         len;
  edg_wlcc_configuration_Error   err;
  char                           buffer[1000], *pc;

  err = edg_wlcc_ConfigurationInitialize( "controller_test.conf", "JobController" );

  if( err ) {
    printf( "Errore: \"%s\"\n", edg_wlcc_ConfigurationError() );

    ret = 1;
  }
  else {
    len = 1000; pc = edg_wlcc_JobController_condor_submit( buffer, &len, &err );
    printf( "CondorSubmit = %s (%d)\n", pc, (int) err );

    len = 1000; pc = edg_wlcc_JobController_condor_remove( buffer, &len, &err );
    printf( "CondorRemove = %s (%d)\n", pc, (int) err );

    len = 1000; pc = edg_wlcc_JobController_condor_query( buffer, &len, &err );
    printf( "CondirQuery = %s (%d)\n", pc, (int) err );

    val = edg_wlcc_JobController_log_level( &err );
    printf( "LogLevel = %d (%d)\n", val, (int) err );

    if( err ) {
      printf( "Errore = \"%s\"\n",  edg_wlcc_ConfigurationError() );

      ret = 1;
    }
  }

  return ret;
}
