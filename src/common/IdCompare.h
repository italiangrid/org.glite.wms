#ifndef EDG_WORKLOAD_JOBCONTROL_COMMON_IDCOMPARE_H
#define EDG_WORKLOAD_JOBCONTROL_COMMON_IDCOMPARE_H

JOBCONTROL_NAMESPACE_BEGIN {

namespace jccommon {

class Compare {
public:
  Compare( void ) {}

  inline bool operator()( const EdgId &uno, const EdgId &due ) { return( uno.edg_id() < due.edg_id() ); }
  inline bool operator()( const EdgId &id, const std::string &str ) { return( id.edg_id() < str ); }
  inline bool operator()( const std::string &str, const EdgId &id ) { return( str < id.edg_id() ); }

  inline bool operator()( const CondorId &uno, const CondorId &due ) { return( uno.condor_id() < due.condor_id() ); }
  inline bool operator()( const CondorId &id, const std::string &str ) { return( id.condor_id() < str ); }
  inline bool operator()( const std::string &str, const CondorId &id ) { return( str < id.condor_id() ); }
};

class CompareWith {
public:
  CompareWith( const std::string &str ) : cw_string( str ) {}

  inline bool operator()( const EdgId &id ) { return( id.edg_id() == this->cw_string ); }
  inline bool operator()( const CondorId &id ) { return( id.condor_id() == this->cw_string ); }

private:
  std::string         cw_string;
};

class PrintId {
public:
  PrintId( void ) {}

  void operator()( const EdgId &id ) { std::cout << '\t' << id.edg_id() << '\n'; }
  void operator()( const CondorId &id ) { std::cout << '\t' << id.condor_id() << '\n'; }
  void operator()( PointerId &id )
  { std::cout << id.edg_id() << ", " << id.condor_id() << std::flush; }
};

} // Namespace jccommon

} JOBCONTROL_NAMESPACE_END

#endif /* EDG_WORKLOAD_JOBCONTROL_COMMON_IDCOMPARE_H */

// Local Variables:
// mode: c++
// End:

