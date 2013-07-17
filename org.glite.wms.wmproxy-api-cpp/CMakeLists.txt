message("*\n*\n*\n* Configuring build for org.glite.wms.wmproxy-api-cpp component\n*\n*\n*")

set(PROJECT_SOURCE_DIR
"${MAIN_PROJECT_SOURCE_DIR}/org.glite.wms.wmproxy-api-cpp")

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
                org.glite.wms.wmproxy-api-cpp VersionArray.txt
                COMMAND
                /bin/awk
                "{print \$2}"
                WORKING_DIRECTORY ${MAIN_PROJECT_SOURCE_DIR}
                TIMEOUT 5
                RESULT_VARIABLE RES
                OUTPUT_VARIABLE PVER
                ERROR_VARIABLE ERR 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
                
EXECUTE_PROCESS(COMMAND
                /bin/grep
                org.glite.wms.wmproxy-api-cpp VersionArray.txt
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
	CHANGES
        DESTINATION ${PREFIX}/usr/share/doc/glite-wms-wmproxy-api-cpp-${PVER}/
)

ADD_CUSTOM_COMMAND(TARGET RPM   
    COMMAND make ARGS install
    COMMAND rm ARGS -f /tmp/glite-wms-wmproxy-api-cpp-${PVER}-${AGE}.${OS}.tar.gz
    COMMAND ${PROJECT_SOURCE_DIR}/maketarball.sh ARGS ${PVER} ${AGE} ${OS}
"${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}"
    COMMAND mkdir ARGS -p ${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SPECS
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SRPMS
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/BUILD
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/RPMS
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SOURCES
    COMMAND mv ARGS /tmp/glite-wms-wmproxy-api-cpp-${PVER}-${AGE}.${OS}.tar.gz
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SOURCES/
    COMMAND sed ARGS -e 's/%{extversion}/${PVER}/g' -e 's/%{extage}/${AGE}/g'
-e 's/%{extdist}/${OS}/g' -e 's/%{extcdate}/${DATE}/g' -e 's/%{extclog}/Bug
fixing/g' < project/glite-wms-wmproxy-api-cpp.spec >
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SPECS/glite-wms-wmproxy-api-cpp.spec
    COMMAND rpmbuild ARGS -ba --define "_topdir `pwd`/../rpmbuild" --define
"extbuilddir ${PROJECT_SOURCE_DIR}/stage"
`pwd`/../rpmbuild/SPECS/glite-wms-wmproxy-api-cpp.spec
  )

add_definitions( -pipe -O2 -Wall -DWITH_OPENSSL -DWITH_DOM -DWITH_GZIP
-DHAVE_CONFIG_H -DWITH_IPV6 -DUSE_GSOAP_PARSER -DWITH_COOKIES -DWITH_GZIP
-I/usr/include)

EXEC_PROGRAM(cat
  ARGS  /etc/redhat-release | /usr/bin/awk '{print $(NF-1)}' | /usr/bin/awk
-F"." '{print $1}'
  OUTPUT_VARIABLE RH_VER)

if("${RH_VER}" STREQUAL "5")
  add_definitions( -DGSOAP_VERSION_NUM=20713 )
endif("${RH_VER}" STREQUAL "5")

if("${RH_VER}" STREQUAL "6")
  add_definitions( -DGSOAP_VERSION_NUM=20716 )
endif("${RH_VER}" STREQUAL "6")


add_subdirectory(src)
add_subdirectory(interface)
