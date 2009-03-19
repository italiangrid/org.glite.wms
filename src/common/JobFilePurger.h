#ifndef EDG_WORKLOAD_JOBCONTROL_COMMON_JOBFILEPURGE_H
#define EDG_WORKLOAD_JOBCONTROL_COMMON_JOBFILEPURGE_H

COMMON_SUBNAMESPACE_CLASS_J(jobid, JobId );

JOBCONTROL_NAMESPACE_BEGIN {

namespace jccommon {

class JobFilePurger {
public:
  JobFilePurger( const glite::wmsutils::jobid::JobId &jobid, bool have_lbproxy, bool isdag);
  JobFilePurger( const glite::wmsutils::jobid::JobId &dagid, bool have_lbproxy, const glite::wmsutils::jobid::JobId &jobid );
  ~JobFilePurger( void );

  void do_purge( bool everything = false );

private:
  bool                    jfp_isDag, jfp_have_lbproxy;
  glite::wmsutils::jobid::JobId    jfp_jobId, jfp_dagId;
};

}; // jccommon namespace

} JOBCONTROL_NAMESPACE_END;

#endif /* EDG_WORKLOAD_JOBCONTROL_COMMON_JOBFILEPURGE_H */

// Local Variables:
// mode: c++
// End:
