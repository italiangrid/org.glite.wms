message("*\n*\n*\n* Configuring build for org.glite.wms.jobsubmission component\n*\n*\n*")

set(PROJECT_SOURCE_DIR "${MAIN_PROJECT_SOURCE_DIR}/org.glite.wms.jobsubmission/")

set(PREFIX "${PROJECT_SOURCE_DIR}/stage")

set(SYSCONFIGDIR "${PREFIX}/etc/")

set(PKG_CONFIG_PATH "${PROJECT_SOURCE_DIR}/org.glite.wms.common/stage/usr/lib64/pkgconfig:${PROJECT_SOURCE_DIR}/org.glite.wms.common/stage/usr/lib/pkgconfig:")

ADD_CUSTOM_TARGET (RPM)

if(EXISTS "/etc/debian_version")
  set(ONDEB "1")
endif(EXISTS "/etc/debian_version")

if(EXISTS "/etc/redhat-release")
  set(ONRH "1")
endif(EXISTS "/etc/redhat-release")

if(DEFINED ONDEB)
  add_definitions( -DGSOAP_279_TRICK )
endif(DEFINED ONDEB)

add_definitions( ${CLASSAD_CFLAGS} -DLDAP_DEPRECATED)

EXECUTE_PROCESS(COMMAND
		/bin/grep
		org.glite.wms.jobsubmission VersionArray.txt
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
		org.glite.wms.jobsubmission VersionArray.txt
		COMMAND
		/bin/awk
		"{print \$3}"
		WORKING_DIRECTORY ${MAIN_PROJECT_SOURCE_DIR}
		TIMEOUT 5
		RESULT_VARIABLE RES
                OUTPUT_VARIABLE AGE
                ERROR_VARIABLE ERR 
		OUTPUT_STRIP_TRAILING_WHITESPACE)

install(FILES
        LICENSE
        DESTINATION ${PREFIX}/usr/share/doc/glite-wms-jobsubmission-${PVER}/
)
add_subdirectory(src)
add_subdirectory(interface)
add_subdirectory(doc)
add_subdirectory(config)
