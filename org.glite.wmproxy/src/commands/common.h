/*
 * common.h
 * 
 * Copyright (c) 2001, 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */
 
#ifndef _GLITE_WMS_WMPROXY_COMMANDS_COMMON_H_
#define _GLITE_WMS_WMPROXY_COMMANDS_COMMON_H_

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {

  class Command;

    /**
     * Compute command's Sandbox Size.
     * This methods gets command jdl string and determines
     * the sandbox size. For a successful execution command
     * must have "jdl" and "Sandbox" attributes. "Sandboxsize"
     * attribute is written.
     * @param cmd a pointer to the command.
     * @return true on success, false otherwise.
     */
    extern bool computeSandboxSize (Command* cmd);
 
    /**
     * Check whether there's enough free space 
     * to transfer the Sandbox on the hard drive.
     * This methods gets sandbox and checks whether
     * there's enough free space on the hard drive to
     * store the entire sandbox. 
     * "CheckPassed" attribute is written.
     * @param cmd a pointer to the command.
     * @return true on success, false otherwise.
     */
    extern bool checkSpace (Command* cmd);

    /**
     * Substitutes the string what inside the string where using
     * "with" string.
     * @param where the source string.
     * @param what the string to be substituted.
     * @param with the string to change with.
     */
    extern void replace(std::string& where, const std::string& what, const std::string& with);

    /**
     * Copies a file.
     * @param from the source path.
     * @param to the destination path.
     * @return true on success, false otherwise.
     */
    extern bool fcopy(const std::string& from, const std::string& to);

} // namespace commands
} // namespace wmproxy
} // namespace wms
} // namespace glite 
 
#endif


