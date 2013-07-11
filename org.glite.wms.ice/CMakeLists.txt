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


install(FILES
        LICENSE
        DESTINATION ${PREFIX}/usr/share/doc/glite-wms-ice-${PVER}/
)


ADD_CUSTOM_COMMAND(TARGET RPM   
    COMMAND make ARGS install
    COMMAND rm ARGS -f /tmp/glite-wms-ice-${PVER}-${AGE}.${OS}.tar.gz
    COMMAND ${PROJECT_SOURCE_DIR}/maketarball.sh ARGS ${PVER} ${AGE} ${OS}
"${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}"
    COMMAND mkdir ARGS -p ${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SPECS
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SRPMS
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/BUILD
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/RPMS
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SOURCES
    COMMAND mv ARGS /tmp/glite-wms-ice-${PVER}-${AGE}.${OS}.tar.gz
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SOURCES/
    COMMAND sed ARGS -e 's/%{extversion}/${PVER}/g' -e 's/%{extage}/${AGE}/g'
-e 's/%{extdist}/${OS}/g' -e 's/%{extcdate}/${DATE}/g' -e 's/%{extclog}/Bug
fixing/g' < project/glite-wms-ice.spec >
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SPECS/glite-wms-ice.spec
    COMMAND rpmbuild ARGS -ba --define "_topdir `pwd`/../rpmbuild" --define
"extbuilddir ${PROJECT_SOURCE_DIR}/stage"
`pwd`/../rpmbuild/SPECS/glite-wms-ice.spec
  )


add_subdirectory(src)
add_subdirectory(config)
