
message("*\n*\n*\n* Configuring build for org.glite.wms.nagios component\n*\n*\n*")

set(PROJECT_SOURCE_DIR "${MAIN_PROJECT_SOURCE_DIR}/org.glite.wms.nagios/")

set(PREFIX "${PROJECT_SOURCE_DIR}/stage")

set(SYSCONFIGDIR "${PREFIX}/etc/")

set(PKG_CONFIG_PATH
"${PROJECT_SOURCE_DIR}/org.glite.wms.common/stage/usr/lib64/pkgconfig:${PROJECT_SOURCE_DIR}/org.glite.wms.common/stage/usr/lib/pkgconfig:")

ADD_CUSTOM_TARGET (RPM)

EXEC_PROGRAM(/bin/mkdir ARGS -p ${PREFIX}/usr)

EXEC_PROGRAM(/bin/echo ARGS \"[global]\" > org.glite.wms.nagios/setup.cfg 2>/dev/null OUTPUT_VARIABLE FIRST)

EXEC_PROGRAM(/bin/echo ARGS \"pkgversion=${PVER}\" >> org.glite.wms.nagios/setup.cfg 2>/dev/null OUTPUT_VARIABLE SECOND)

EXEC_PROGRAM(cd ARGS nagios; ${PYTHON_EXECUTABLE} setup.py install -O1 --prefix ${PREFIX}/usr --install-data ${PREFIX}; cd - OUTPUT_VARIABLE THIRD)

add_custom_target(nagios)
	
EXECUTE_PROCESS(COMMAND
                /bin/grep
                org.glite.wms.nagios VersionArray.txt
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
                org.glite.wms.nagios VersionArray.txt
                COMMAND
                /bin/awk
                "{print \$3}"
                WORKING_DIRECTORY ${MAIN_PROJECT_SOURCE_DIR}
                TIMEOUT 5
                RESULT_VARIABLE RES
                OUTPUT_VARIABLE AGE
                ERROR_VARIABLE ERR 
                OUTPUT_STRIP_TRAILING_WHITESPACE)

ADD_CUSTOM_COMMAND(TARGET RPM   
    COMMAND make ARGS install
    COMMAND rm ARGS -f /tmp/emi-wms-nagios-${PVER}-${AGE}.${OS}.tar.gz
    COMMAND ${PROJECT_SOURCE_DIR}/maketarball.sh ARGS ${PVER} ${AGE} ${OS}
"${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}"
    COMMAND mkdir ARGS -p ${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SPECS
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SRPMS
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/BUILD
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/RPMS
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SOURCES
    COMMAND mv ARGS /tmp/emi-wms-nagios-${PVER}-${AGE}.${OS}.tar.gz
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SOURCES/
    COMMAND sed ARGS -e 's/%{extversion}/${PVER}/g' -e 's/%{extage}/${AGE}/g'
-e 's/%{extdist}/${OS}/g' -e 's/%{extcdate}/${DATE}/g' -e 's/%{extclog}/Bug
fixing/g' < project/emi-wms-nagios.spec >
${MAIN_PROJECT_SOURCE_DIR}/rpmbuild/SPECS/emi-wms-nagios.spec
    COMMAND rpmbuild ARGS -ba --define "_topdir `pwd`/../rpmbuild" --define
"extbuilddir ${PROJECT_SOURCE_DIR}/stage"
`pwd`/../rpmbuild/SPECS/emi-wms-nagios.spec
  )


add_custom_command(TARGET nagios
  COMMAND 
  mkdir -p ${PREFIX}/usr
  COMMAND
  /bin/echo \"[global]\" > setup.cfg
  COMMAND
  /bin/echo \"pkgversion=${PVER}\" >> setup.cfg
  COMMAND
  /usr/bin/python setup.py install -O1 --prefix ${PREFIX}/usr --install-data
${PREFIX}/usr
  )
