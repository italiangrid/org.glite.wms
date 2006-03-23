/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
/*
 * File: wmp2wm.h
 * Author: Marco Pappalardo
 */

#ifndef GLITE_WMS_WMPROXY_SERVER_WMP2WM_H
#define GLITE_WMS_WMPROXY_SERVER_WMP2WM_H

#include <string>

// Boost
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

// FileList
#include "glite/wms/common/utilities/FileList.h"
#include "glite/wms/common/utilities/FileListLock.h"

// Eventlogger
#include "eventlogger/wmpeventlogger.h"

namespace classad {
  class ClassAd;
}

class WMPLogger;

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {
  class Command;
}

namespace server {

typedef boost::scoped_ptr<glite::wms::common::utilities::FileList<std::string> >
	FileListPtr;
typedef boost::scoped_ptr<glite::wms::common::utilities::FileListMutex>
	FileListMutexPtr;


/**
 * WMP2WM class
 *
 * @version 1.0
 * @date 2004
 * @author Giuseppe Avellino <giuseppe.avellino@datamat.it>
*/
class WMP2WM
{
 	FileListPtr m_filelist;
	FileListMutexPtr m_mutex;

public:
	/**
	 * Contructor
	 */
	WMP2WM();
  
	/**
	 *  Destructor
	 */
	virtual ~WMP2WM();

	/**
	 * Initialized the WMP to WM Proxy using the given FileList.
     * @param filename the pathname of FileList file.
     * @param wmpeventlogger
     */
	void init(const std::string &filename, 
  		glite::wms::wmproxy::eventlogger::WMPEventLogger *wmpeventlogger);

	/**
   	 * Forwards a submit command to the WM through the FileList.
     * @param cmdAd the Command object related to the edg_job_submit as
     * provided by WMP.
     */
	virtual void submit(const std::string &jdl, const std::string &jdlpath = "");
  
	/**
     * Forwards a cancel command to the WM through the FileList.
     * @param cmdAd the Command object related to the edg_job_cancel as
     * provided by WMP.
     */  
	virtual void cancel(const std::string &jobid, const std::string &seq_code);
  
	/**
     * Forwards a match command to the WM through the FileList.
     * @param cmdAd the Command object related to the edg_job_list_match
     * as provided by WMP.
     */  
	virtual void match(const std::string &jdl, const std::string &file,
		const std::string &proxy, void * result);

private:

	/**
     * Stores the sequence code for Purger activities, in case of submit.
     * @param cmdAd the Command classad object related to the edg_job_submit
     * as provided by WMP.
     */
	void storeSequenceCode(classad::ClassAd* cmdAd);
	
	/**                                    
     * Stores sequence codes for DAG Nodes.                       
  	 * @param classad the dag classad.                              
   	 * @param seqfile the sequence code file of the node.         
   	 * @param to_root_path the sandbox root path.                 
   	 */ 
  	void storeDAGSequenceCode(classad::ClassAd* classad, std::string seqfile,
  		std::string to_root_path); 
  
	/**
     * Converts WMP inner protocol-based info into conversation protocol common
     * to WMP and WM.
     * @param cmdAd the Command object related to the command as provided by WMP.
     * @return the converted common-protocol-based string.
	 */
	std::string convertProtocol(classad::ClassAd* cmdAd);
  
	// Event logger instance
	glite::wms::wmproxy::eventlogger::WMPEventLogger * wmpeventlogger;
};

} // namespace server
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_WMPROXY_SERVER_WMP2WM_H





