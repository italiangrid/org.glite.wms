
#include "jnlFileManager.h"
#include "glite/ce/cream-client-api-c/string_manipulation.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cerrno>
#include <iostream>

extern int errno;

using namespace glite::wms::ice::util;
using namespace glite::ce::cream_client_api::util;
using namespace std;

//______________________________________________________________________________
jnlFileManager::jnlFileManager(const string& file) 
  throw(jnlFile_ex&) : filename(file), 
		       readonly(true), 
		       isempty(false), 
		       savedoff(0)
{
  /**
   * The file is put in readonly mode, so only getNextOperation can be
   * performed on it; before modifying the journal file it must be put 
   * in readonly OFF mode (by calling readonly(false))
   */
  struct stat stat_buf;
  if(-1==::stat(filename.c_str(), &stat_buf)) {
    if(errno!=ENOENT) {
      throw jnlFile_ex(strerror(errno));
    } else {
      os.open(filename.c_str(), ios::out);
      if(!os)
	throw jnlFile_ex("Error creating an empty journal file");
      os.close();
      isempty=true;
    }
  }

  if(!stat_buf.st_size) isempty=true;

  is.open(filename.c_str(), ios::in);
  if(!is)
    throw jnlFile_ex("Error opening journal file for read");
  
  pthread_mutex_init(&mutexJnlFile, NULL);
}

//______________________________________________________________________________
void jnlFileManager::truncate(void) throw(jnlFile_ex&, jnlFileReadOnly_ex&) 
{
  /**
   * Before to call this method the journal file must be NOT put in readonly 
   * mode by calling the readonly(false) method
   *
   */
  if(readonly) 
    throw jnlFileReadOnly_ex("Cannot truncate: the file is in readonly mode");

  is.close();
  string tmpName = filename+"."+string_manipulation::make_string(::getpid());
  cerr << "Creating an empty file ["<<tmpName<<"]"<<endl;
  os.open(tmpName.c_str(), ios::out);
  if(!os)
    throw jnlFile_ex("Error truncating journal file, Step 1: creating an empty file");
  os.close();
  if(-1==::rename(tmpName.c_str(), filename.c_str()))
    throw jnlFile_ex(strerror(errno));

  os.open(filename.c_str(), ios::app);
  if(!os)
    throw jnlFile_ex("Error truncating journal file, Step 2: renaming");

  isempty = true;
  savedoff = 0;
}

//______________________________________________________________________________
void jnlFileManager::log(const operation& op, const string& param)
  throw(jnlFile_ex&, jnlFileReadOnly_ex&) 
{
  /**
   * Before to call this method the journal file must be NOT put in readonly 
   * mode by calling the readonly(false) method
   *
   */
  if(readonly) throw jnlFileReadOnly_ex("Cannot append: journal file is in readonly mode");//return;
  
  string toWrite = string_manipulation::make_string((int)op) + param;
  os << toWrite << endl;

  if( (!os.good()) || (os.bad()) )
    throw jnlFile_ex("Error logging the operation to journal file");

  isempty = false;
}

//______________________________________________________________________________
void jnlFileManager::readonly_mode(const bool& flag) 
  throw (jnlFile_ex&)
{
  /**
   * flag == true: Closes the output stream linked to the journal file
   *               opens the journal file in readonly mode and 
   *               seeks until the file position saved when last 
   *               readonly_mode(false) was called
   *
   * flag == false: Closes the input stream linked to the journal file,
   *                saves current file position and
   *                opens the journal file in append mode (offset=end of file)
   *
   */
  if(flag) {
    if(readonly) return;
    os.close();
    if(os.fail())
      throw jnlFile_ex("Error closing journal");
    is.open(filename.c_str(), ios::in);

    if(!is)
      throw jnlFile_ex("Error putting journal in readonly mode");
    readonly = true;
    is.seekg(savedoff);
    return;
  } else {
    if(!readonly) return;
    savedoff = is.tellg();
    is.close();
    if(is.fail())
      throw jnlFile_ex("Error closing journal");
    os.open(filename.c_str(), ios::app);

    if(!os)
      throw jnlFile_ex("Error putting journal in readonly mode");
    readonly=false;
    return;
  }
}

//______________________________________________________________________________
bool jnlFileManager::getNextOperation(string& line) 
  throw(jnlFile_ex&, jnlFileReadOnly_ex&) 
{
  /**
   * Before to call this method the journal file must be put in readonly mode
   * by calling the readonly(true) method
   *
   */
  if(!readonly) throw jnlFileReadOnly_ex("Journal file must be NOT put in readonly mode");
  if(isempty) return false;
  if(is.peek()==EOF) return false;

  std::getline(is, line, '\n');

  if(is.fail() || is.bad())
    throw jnlFile_ex("Error reading from journal file");

  if(line == "") return false;

  return true;
}

//______________________________________________________________________________
void jnlFileManager::rewind() throw (jnlFile_ex&)
{
  /**
   * Can rewind the file because one could want to reload the entire file
   * but this has no sense in case of the file is NOT in readonly mode
   * because a journal file in write mode means appending infos at the end of
   * it.
   */
  if(readonly)
    is.seekg(0);
}

//______________________________________________________________________________
void jnlFileManager::lock(void) {
  pthread_mutex_lock(&mutexJnlFile);
}

//______________________________________________________________________________
void jnlFileManager::unlock(void) {
  pthread_mutex_unlock(&mutexJnlFile);
}

