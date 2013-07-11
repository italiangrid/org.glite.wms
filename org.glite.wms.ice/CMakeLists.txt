message("*\n*\n*\n* Configuring build for org.glite.wms.ice component\n*\n*\n*")

set(PROJECT_SOURCE_DIR "${MAIN_PROJECT_SOURCE_DIR}/org.glite.wms.ice/")

set(PREFIX "${PROJECT_SOURCE_DIR}/stage")

set(SYSCONFIGDIR "${PREFIX}/etc/")

set(PKG_CONFIG_PATH
"${PROJECT_SOURCE_DIR}/org.glite.wms.common/stage/usr/lib64/pkgconfig:${PROJECT_SOURCE_DIR}/org.glite.wms.common/stage/usr/lib/pkgconfig:")

ADD_CUSTOM_TARGET (RPM)

if(EXISTS "/etc/redhat-release")
  set(ONRH "1")
endif(EXISTS "/etc/redhat-release")

if(DEFINED ONRH)
EXEC_PROGRAM(cat
  ARGS  /etc/redhat-release | /usr/bin/awk '{print $(NF-1)}' | /usr/bin/awk -F"." '{print $1}'
  OUTPUT_VARIABLE RH_VER)
endif(DEFINED ONRH)

add_definitions( -DICE_VERSIONID="${ICE_VERSIONID}" -DHAVE_GLITE_JOBID -DGLITE_WMS_ICE_ENABLE_STATS -UHAVE_CONFIG_H  ${CLASSAD_CFLAGS} )

EXECUTE_PROCESS(COMMAND
                /bin/grep
                org.glite.wms.ice VersionArray.txt
                COMMAND
                /bin/awk
                "{print \$2}"
                WORKING_DIRECTORY ${MAIN_PROJECT_SOURCE_DIR}
                TIMEOUT 5
                RESULT_VARIABLE RES
                OUTPUT_VARIABLE PVER
                ERROR_VARIABLE ERR 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
                
#message(WARNING " RES=${RES} PVER=${PVER} ERR=${ERR}")
        

EXECUTE_PROCESS(COMMAND
                /bin/grep
                org.glite.wms.ice VersionArray.txt
                COMMAND
                /bin/awk
                "{print \$3}"
                WORKING_DIRECTORY ${MAIN_PROJECT_SOURCE_DIR}
                TIMEOUT 5
                RESULT_VARIABLE RES
                OUTPUT_VARIABLE AGE
                ERROR_VARIABLE ERR 
                OUTPUT_STRIP_TRAILING_WHITESPACE)


add_subdirectory(src)
add_subdirectory(config)
