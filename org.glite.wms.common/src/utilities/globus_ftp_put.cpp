/* File: globus_ftp_put.cpp
 * Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#include <iostream>
#include <globus_ftp_client.h>

#include "logger/edglog.h"
#include "logger/manipulators.h"
#include "utilities/globus_ftp_macros.h"

#define MAX_BUFFER_SIZE 2048

namespace glite {
namespace wms {
namespace common {
namespace utilities {
namespace globus {
	
namespace logger = glite::wms::common::logger;

namespace {

static globus_mutex_t lock;
static globus_cond_t cond;
static globus_bool_t done;

static void done_cb(void * user_arg, globus_ftp_client_handle_t *handle, globus_object_t *err)
{
    if(err)
    {
       edglog(warning) << globus_object_printable_to_string(err) << std::flush;
    }
    globus_mutex_lock(&lock);
    done = GLOBUS_TRUE;
    globus_cond_signal(&cond);
    globus_mutex_unlock(&lock);
    return;
}

static void data_cb(void *user_arg, globus_ftp_client_handle_t *handle, globus_object_t *err,
    globus_byte_t *buffer,
    globus_size_t  length,
    globus_off_t   offset,
    globus_bool_t  eof)
{
    if(err) {
        edglog(warning) << globus_object_printable_to_string(err) << std::flush;
    }
    else {
        if(!eof) {
            FILE *fd = (FILE *) user_arg;
            int rc;
            rc = fread(buffer, 1, MAX_BUFFER_SIZE, fd);
            if (ferror(fd) != 0) {
		edglog(warning) << "Read error in data call-back function: errno = " << errno << std::endl;
                return;
            } 
            globus_ftp_client_register_write(
                handle,
                buffer,
                rc,
                offset + length,
                feof(fd) != 0,
                data_cb,
                (void *) fd);
        } /* if(!eof) */
    } /* else */
    return;
} 

} // namespace anonymous

bool put(const std::string& src, const std::string& dst) 
{
    globus_ftp_client_handle_t              handle;
    globus_byte_t                           buffer[MAX_BUFFER_SIZE];
    globus_size_t                           buffer_length = MAX_BUFFER_SIZE;
    globus_result_t                         result;

    FILE *fd = fopen(src.c_str(), "r");
    if (!fd) {
	edglog(warning) << "Error opening local file: " << src << std::endl;
        return false;
    }
    
//    globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
    globus_mutex_init(&lock, GLOBUS_NULL);
    globus_cond_init(&cond, GLOBUS_NULL);
    globus_ftp_client_handle_init(&handle,  GLOBUS_NULL);

    done = GLOBUS_FALSE;
    
    result = globus_ftp_client_put(&handle,
                                   dst.c_str(),
                                   GLOBUS_NULL,
                                   GLOBUS_NULL,
                                   done_cb,
                                   0);
    if(result != GLOBUS_SUCCESS) {
        globus_object_t * err;
        err = globus_error_get(result);
        edglog(warning) << globus_object_printable_to_string(err) << std::flush;
        done = GLOBUS_TRUE;
    }
    else {
	int rc;
        rc = fread(buffer, 1, buffer_length, fd);
        globus_ftp_client_register_write(
            &handle,
            buffer,
            rc,
            0,
            feof(fd) != 0,       
            data_cb,
            (void *) fd);
    }

    globus_mutex_lock(&lock);
    while(!done)
    {
        globus_cond_wait(&cond, &lock);
    }
    globus_mutex_unlock(&lock);

    globus_ftp_client_handle_destroy(&handle);
//    globus_module_deactivate_all();

    return result == GLOBUS_SUCCESS;
}
} // globus namespace closure
} // utilities namespace closure
} // common namespace closure
} // wms namespace closure
} // glite namespace closure

