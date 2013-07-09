
message("*\n*\n*\n* Configuring build for org.glite.wms.common component\n*\n*\n*")

set(PROJECT_SOURCE_DIR "${MAIN_PROJECT_SOURCE_DIR}/org.glite.wms.common/")

set(PREFIX "${PROJECT_SOURCE_DIR}/stage")

set(SYSCONFIGDIR "${PREFIX}/etc/")

ADD_CUSTOM_TARGET (RPM)

EXECUTE_PROCESS(COMMAND
		/bin/grep
		org.glite.wms.common VersionArray.txt
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
		org.glite.wms.common VersionArray.txt
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
        DESTINATION ${PREFIX}/usr/share/doc/glite-wms-common-${PVER}/
)

ADD_CUSTOM_COMMAND(TARGET RPM   
    COMMAND make ARGS install
    COMMAND rm ARGS -f /tmp/glite-wms-common-${PVER}-${AGE}.${OS}.tar.gz
    COMMAND ${PROJECT_SOURCE_DIR}/maketarball.sh ARGS ${PVER} ${AGE} ${OS} "${PROJECT_SOURCE_DIR}"
${PROJECT_SOURCE_DIR}
    COMMAND mkdir ARGS -p ${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SPECS ${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SRPMS ${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/BUILD ${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/RPMS ${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SOURCES
    COMMAND mv ARGS /tmp/glite-wms-common-${PVER}-${AGE}.${OS}.tar.gz ${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SOURCES/
    COMMAND sed ARGS -e 's/%{extversion}/${PVER}/g' -e 's/%{extage}/${AGE}/g' -e 's/%{extdist}/${OS}/g' -e 's/%{extcdate}/${DATE}/g' -e 's/%{extclog}/Bug fixing/g' < project/glite-wms-common.spec > ${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SPECS/glite-wms-common.spec
    COMMAND rpmbuild ARGS -ba --define "_topdir `pwd`/../rpmbuild" --define "extbuilddir ${PROJECT_SOURCE_DIR}/stage" `pwd`/../rpmbuild/SPECS/glite-wms-common.spec
  )

add_definitions(${CLASSAD_CFLAGS})
  
add_subdirectory(src)
add_subdirectory(interface)
