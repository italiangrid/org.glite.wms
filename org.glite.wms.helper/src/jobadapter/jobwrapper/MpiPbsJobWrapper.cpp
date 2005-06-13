/***************************************************************************
 *  filename  : MpiPbsJobWrapper.cpp
 *  authors   : Alessio Gianelle <alessio.gianelle@pd.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
 *              Rosario Peluso <rosario.peluso@pd.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#include <algorithm>
#include <cassert>

#include "JobWrapper.h"
#include "MpiPbsJobWrapper.h"

using namespace std;

namespace url = glite::wms::helper::jobadapter::url;

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {
namespace jobwrapper {

MpiPbsJobWrapper::MpiPbsJobWrapper(const string& job)
  : JobWrapper(job)
{ 
}

MpiPbsJobWrapper::~MpiPbsJobWrapper(void)
{
}

ostream&
MpiPbsJobWrapper::set_subdir(ostream& os,
                             const string& dir) const
{
  os << "newdir=\"" << dir << "\"" << endl
     << "mkdir -p \".mpi/\"${newdir}" << endl
     << "if [ $? != 0 ]; then" << endl
     << "  echo \"Cannot create \".mpi/\"${newdir} directory\"" << endl << endl;

  os << "  "; this->set_lb_sequence_code(os,
		  "Done",
		  "Cannot create \".mpi/\"${newdir} directory",
		  "FAILED",
		  "0");

  return os << "  exit 1" << endl
            << "fi" << endl
            << "cd \".mpi/\"${newdir}" << endl
            << endl;
}

ostream& 
MpiPbsJobWrapper::execute_job(ostream& os,
		 const string& arguments,
		 const string& job,
		 const string& stdi,
		 const string& stdo,
		 const string& stde,
		 int           node) const
{
  os << "HOSTFILE=${PBS_NODEFILE}" << endl
     << endl;

  os << "for i in `cat $HOSTFILE`; do" << '\n'
     << "  ssh $i mkdir -p `pwd`" << '\n'
     << "  /usr/bin/scp -rp ./* $i:`pwd`" << '\n'
     << "  ssh $i chmod 755 `pwd`/" << job << '\n'
     << "done" << '\n'
     << endl;

  if (arguments != "") {
    os << "mpirun -np " << node << " -machinefile ${HOSTFILE} "
       << "  \"" << job << "\"" << " " << arguments << " $*";
  } else {
    os << "mpirun -np " << node << " -machinefile ${HOSTFILE} "
       << "  \"" << job << "\" $*";
  }

  if (stdi != "") {
    os << " < \"" << stdi << "\"";
  }

  if (stdo != "") {
    os << " > \"" << stdo << "\"";
  } else {
    os << " 2> /dev/null";
  }

  if (stde != "") {
    os << " 2> \"" << stde << "\"";
  } else {
    os << " 2> /dev/null";
  }

  return os << endl 
	    << endl;  
}

} // namespace jobwrapper
} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite
