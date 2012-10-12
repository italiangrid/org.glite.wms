#!/bin/bash

autotools_build()
{
   COMPONENT=$1
   VERSION=$2
   AGE=$3
   PACKAGE_NAME=$4
   LOCAL_STAGE_DIR=$5

   cd $COMPONENT
   mkdir -p build src/autogen rpmbuild/SOURCES 2>/dev/null
   cp -R $BUILD_DIR/org.glite.wms/org.glite.wms-ui/project/doxygen project/doxygen 2>/dev/null # needed by some UI component
   aclocal -I ${M4_LOCATION} && \
	libtoolize --force && autoheader && automake --foreign --add-missing --copy && \
	autoconf
   if [ $? -ne 0 ]; then
      echo ERROR
      exit
   fi

   # create the source tarball before configure and make
   tar --exclude reports --exclude rpmbuild --exclude build --exclude bin --exclude tools -zcf \
      ./rpmbuild/SOURCES/${PACKAGE_NAME}-${VERSION}-${AGE}.${PLATFORM}.tar.gz .
   if [ $? -ne 0 ]; then
     echo ERROR
     exit
   fi
   cd build
   ../configure --prefix=${LOCAL_STAGE_DIR}/usr --sysconfdir=${LOCAL_STAGE_DIR}/etc --disable-static PVER=${VERSION}
   if [ $? -ne 0 ]; then
      echo ERROR
      exit
   fi
   make
   if [ $? -ne 0 ]; then
      echo ERROR
      exit
   fi
   make doxygen-doc >/dev/null 2>&1 # only needed by some UI component
   make install
   if [ $? -ne 0 ]; then
      echo ERROR
      exit
   fi
   mkdir -p ${LOCAL_STAGE_DIR}/usr/share/doc/${PACKAGE_NAME}
   cp -r autodoc/html ${LOCAL_STAGE_DIR}/usr/share/doc/${PACKAGE_NAME} 2>/dev/null # needed by some UI component
   cd .. # from build to component root
   rpm_package $VERSION $AGE $PLATFORM $PACKAGE_NAME $COMPONENT $LOCAL_STAGE_DIR
   mv ./rpmbuild/SOURCES/${PACKAGE_NAME}-${VERSION}-${AGE}.${PLATFORM}.tar.gz "$BUILD_DIR"/org.glite.wms/tgz
   cd $BUILD_DIR/org.glite.wms
}

cmake_build()
{
   COMPONENT=$1
   VERSION=$2
   AGE=$3
   PACKAGE_NAME=$4
   LOCAL_STAGE_DIR=$5

   cd $COMPONENT
   echo TODO
   cd $BUILD_DIR/org.glite.wms
}

ant_build()
{
   COMPONENT=$1
   VERSION=$2
   AGE=$3
   PACKAGE_NAME=$4
   LOCAL_STAGE_DIR=$5

   cd $COMPONENT
   mkdir -p lib bin autogen doc/autogen src/autogen ${LOCAL_STAGE_DIR}/usr/share/doc/ rpmbuild/SOURCES
   tar --exclude rpmbuild --exclude ${LOCAL_STAGE_DIR} --exclude --exclude bin --exclude tools -zcf \
      ./rpmbuild/SOURCES/${PACKAGE_NAME}-${VERSION}-${AGE}.${PLATFORM}.tar.gz .
   if [ $? -ne 0 ]; then
      echo ERROR
      exit
   fi
   echo "dist.location=${LOCAL_STAGE_DIR}" > .configuration.properties
   echo "org.glite.wms.wsdl.location=$BUILD_DIR/org.glite.wms/org.glite.wms.wmproxy/src/server" >> .configuration.properties
   echo "module.version=${VERSION}" >> .configuration.properties
   ant
   if [ $? -ne 0 ]; then
      echo ERROR
      exit
   fi
   ant install
   if [ $? -ne 0 ]; then
      echo ERROR
      exit
   fi
   rpm_package $VERSION $AGE $PLATFORM $PACKAGE_NAME $COMPONENT $LOCAL_STAGE_DIR
   mv ./rpmbuild/SOURCES/${PACKAGE_NAME}-${VERSION}-${AGE}.${PLATFORM}.tar.gz "$BUILD_DIR"/org.glite.wms/tgz
   cd $BUILD_DIR/org.glite.wms
}

python_build()
{
   COMPONENT=$1
   VERSION=$2
   AGE=$3
   PACKAGE_NAME=$4
   LOCAL_STAGE_DIR=$5

   cd $COMPONENT
   mkdir -p rpmbuild/SOURCES
   tar --exclude rpmbuild --exclude build --exclude bin --exclude tools -zcf \
      ./rpmbuild/SOURCES/${PACKAGE_NAME}-${VERSION}-${AGE}.${PLATFORM}.tar.gz .
   if [ $? -ne 0 ]; then
      echo ERROR
      exit
   fi
   echo "[global]" > setup.cfg 2>/dev/null
   echo "pkgversion=${VERSION}" >> setup.cfg 2>/dev/null
   python setup.py install -O1 --prefix ${LOCAL_STAGE_DIR}/usr --install-data ${LOCAL_STAGE_DIR}
   rpm_package $VERSION $AGE $PLATFORM $PACKAGE_NAME $COMPONENT $LOCAL_STAGE_DIR
   mv ./rpmbuild/SOURCES/${PACKAGE_NAME}-${VERSION}-${AGE}.${PLATFORM}.tar.gz "$BUILD_DIR"/org.glite.wms/tgz
   cd $BUILD_DIR/org.glite.wms
}

no_build()
{
   COMPONENT=$1
   VERSION=$2
   AGE=$3
   PACKAGE_NAME=$4

   cd $COMPONENT
   mkdir -p rpmbuild/SOURCES 2>/dev/null
   # creating binary tarball
   tar --exclude project --exclude rpmbuild \
      -czf rpmbuild/SOURCES/${PACKAGE_NAME}-$VERSION-$AGE.tar.gz *
   if [ $? -ne 0 ]; then
     echo ERROR
     exit
   fi
   eval "sed -e 's/__version__/$VERSION/g' -e 's/__release__/$AGE/g' \
      < project/$PACKAGE_NAME.spec.in > project/$PACKAGE_NAME.spec"
   rpmbuild -bb --target $ARCH --define "_topdir $BUILD_DIR/org.glite.wms/$COMPONENT/rpmbuild" \
      project/$PACKAGE_NAME.spec
   if [ $? -ne 0 ]; then
     echo ERROR
     exit
   fi
   mv ./rpmbuild/SOURCES/${PACKAGE_NAME}-${VERSION}-${AGE}.${PLATFORM}.tar.gz "$BUILD_DIR"/org.glite.wms/tgz
   cd $BUILD_DIR/org.glite.wms
}

rpm_package()
{
   VERSION=$1
   AGE=$2
   PLATFORM=$3
   PACKAGE_NAME=$4
   COMPONENT=$5
   LOCAL_STAGE_DIR=$6

   mkdir rpmbuild/SPECS rpmbuild/SRPMS rpmbuild/BUILD rpmbuild/RPMS 2>/dev/null
   eval "sed -e 's/%{extversion}/$VERSION/g' -e 's/%{extage}/$AGE/g' \
      -e 's/%{extdist}/$PLATFORM/g' -e 's/%{extcdate}/`date +'%a %b %d %Y'`/g' \
      -e 's/%{extclog}/Bug fixing/g' \
      < project/$PACKAGE_NAME.spec > rpmbuild/SPECS/${PACKAGE_NAME}.spec"
   rpmbuild -ba --define "_topdir ${BUILD_DIR}/org.glite.wms/$COMPONENT/rpmbuild" \
      --define "extbuilddir $LOCAL_STAGE_DIR" \
      rpmbuild/SPECS/${PACKAGE_NAME}.spec
   if [ $? -ne 0 ]; then
      echo ERROR
      exit
   fi

   /usr/bin/rpmlint --file=/usr/share/rpmlint/config -i rpmbuild/RPMS/$ARCH/*
   /usr/bin/rpmlint --file=/usr/share/rpmlint/config -i rpmbuild/SRPMS/*

   mv rpmbuild/RPMS/$ARCH/* "$BUILD_DIR"/org.glite.wms/RPMS
   mv rpmbuild/SRPMS/* "$BUILD_DIR"/org.glite.wms/SRPMS
}

get_external_deps()
{
   # EPEL repositories
   if [ $PLATFORM = "sl6" ]; then
     sudo rpm -ivh http://ftp.tu-chemnitz.de/pub/linux/fedora-epel/6/i386/epel-release-6-7.noarch.rpm
   elif [ $PLATFORM = "sl5" ]; then
      sudo rpm -ivh http://fedora.uib.no/epel/5/i386/epel-release-5-4.noarch.rpm
   fi
   # EMI repositories
   sudo rpm --import http://emisoft.web.cern.ch/emisoft/dist/EMI/$EMI_RELEASE/RPM-GPG-KEY-emi
   sudo rpm -ivh "http://emisoft.web.cern.ch/emisoft/dist/EMI/$EMI_RELEASE/sl6/x86_64/base/emi-release-${EMI_RELEASE}.0.0-1.$PLATFORM.noarch.rpm"

   sudo yum -y install ${DEPS_LIST[@]}
}

#
# MAIN
#

if [ -z $8 ]; then
   echo "wms <tag:master/...> <build-dir-name> <emi-release:1/2/3> <os:sl5/sl6/deb6> <want_external_deps:0/1> <want_vcs_checkout:0/1> <want_cleanup:0/1> <want_mock:0/1>"
   exit
fi

BUILD_DIR=`pwd`/$2
EMI_RELEASE=$3
PLATFORM=$4
# TODO
#if [ $PLATFORM == "deb" ]; then
#   PACKAGER=rpm
#else
#   PACKAGER=deb
#fi
M4_LOCATION=/usr/share/emi/build/m4
ARCH=`uname -i`
DEPS_LIST=( yum-priorities pkgconfig mock rpmlint mod_fcgid mod_ssl axis2 gridsite-devel httpd-devel zlib-devel boost-devel c-ares-devel glite-px-proxyrenewal-devel voms-devel voms-clients argus-pep-api-c-devel lcmaps-without-gsi-devel lcmaps-devel classads-devel glite-build-common-cpp gsoap-devel libtar-devel cmake globus-ftp-client globus-ftp-client-devel log4cpp-devel log4cpp glite-jobid-api-c glite-jobid-api-c-devel glite-jobid-api-cpp-devel openldap-devel python-ldap glite-wms-utils-exception glite-wms-utils-classad glite-wms-utils-exception-devel glite-wms-utils-classad-devel chrpath cppunit-devel glite-jdl-api-cpp-devel glite-lb-client-devel glite-lbjp-common-gsoap-plugin-devel condor-emi glite-ce-cream-client-api-c glite-ce-cream-client-devel emi-trustmanager emi-trustmanager-axis )

# mock build
if [ $8 -eq 1 ]; then
   echo -e "\n*** starting mock build for `ls -1 $BUILD_DIR/org.glite.wms/SRPMS/` ***\n"

   # file /etc/mock/emi${EMI_RELEASE}-$PLATFORM-$ARCH.cfg must be created, with all the required repositories and stuff
   if [ ! -r /etc/mock/emi${EMI_RELEASE}-$PLATFORM-$ARCH.cfg ]; then
      echo "Do you also want me to prepare the mock configuration?!? What else?"
      exit
   fi

   if [ $5 -eq 1 ]; then
      echo -e "\n*** wiping out the mock environment ***\n"
      mock -r emi${EMI_RELEASE}-$PLATFORM-$ARCH --clean
      echo -e "\n*** one time initialization of the mock environment ***\n"
      mock -r emi${EMI_RELEASE}-$PLATFORM-$ARCH --init
      echo -e "\n*** installing external dependencies in mock - hang on, this may take very long ***\n"
      mock -r emi${EMI_RELEASE}-$PLATFORM-$ARCH --install ${DEPS_LIST[@]}
      # same as: yum --installroot /var/lib/mock/sl6-emi-2-x86_64/root install ${DEPS_LIST[@]}
      # while rpm has a similar option --root
   fi

   for pkgname in ${PACKAGE_NAME[@]}; do
      mock -r emi${EMI_RELEASE}-$PLATFORM-$ARCH --rebuild \
         "$BUILD_DIR/org.glite.wms/SRPMS/$pkgname-${VERSION}-${AGE}.${PLATFORM}.src.rpm"
      rpm --root /var/lib/mock/emi${EMI_RELEASE}-$PLATFORM-$ARCH/root -ivh \
         /var/lib/mock/emi${EMI_RELEASE}-$PLATFORM-$ARCH/result/$pkgname-*.rpm
   done

   echo -e "\n*** mock build completed ***\n"
   exit
fi

if [ $5 -eq 1 ]; then
   echo -e "\n*** installing external dependencies ***\n"
   get_external_deps
fi

echo -e "\n*** build dir: $BUILD_DIR, platform: $PLATFORM, architecture: $ARCH ***\n"

if [ $6 -ne 0 ]; then
   sudo rm -rf "$BUILD_DIR"
   mkdir -p "$BUILD_DIR"
   cd "$BUILD_DIR"
   echo -e "\n*** checking out the WMS project ***\n"
   git clone --progress -v git@github.com:MarcoCecchi/org.glite.wms.git
   git checkout $1
   cd org.glite.wms
else
   cd "$BUILD_DIR"/org.glite.wms
   echo -e "\n*** NOT checking out the WMS project ***\n"
fi
mkdir tgz RPMS SRPMS 2>/dev/null

echo -e "\n*** starting build ***\n"

COMPONENT=( org.glite.wms.common org.glite.wms.ism org.glite.wms.helper org.glite.wms.purger org.glite.wms.jobsubmission org.glite.wms.manager org.glite.wms.wmproxy org.glite.wms.ice org.glite.wms.nagios org.glite.wms org.glite.wms.brokerinfo-access org.glite.wms.wmproxy-api-cpp org.glite.wms.wmproxy-api-java org.glite.wms.wmproxy-api-python org.glite.wms-ui.api-python org.glite.wms-ui.commands )
BUILD_TYPE=( autotools autotools autotools autotools autotools autotools autotools autotools null pkg_only autotools autotools ant python autotools autotools )
PACKAGE_NAME=( glite-wms-common glite-wms-ism glite-wms-helper glite-wms-purger glite-wms-jobsubmission glite-wms-manager glite-wms-wmproxy glite-wms-ice emi-wms-nagios emi-wms glite-wms-brokerinfo-access glite-wms-wmproxy-api-cpp glite-wms-wmproxy-api-java glite-wms-wmproxy-api-python glite-wms-ui-api-python glite-wms-ui-commands )
VERSION=( 3.5.0 3.5.0 3.5.0 3.5.0 3.5.0 3.5.0 3.5.0 3.5.0 3.5.0 3.5.0 3.5.0 3.5.0 3.5.0 3.5.0 3.5.0 3.5.0 )
AGE=( 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 )
START=0
END=15

export PKG_CONFIG_PATH=$BUILD_DIR/org.glite.wms/org.glite.wms/project/ # for condor-g.pc and maybe others
if [ -d /usr/lib64 ]; then
   LOCAL_PKGCFG_LIB=usr/lib64/pkgconfig/
else
   LOCAL_PKGCFG_LIB=usr/lib/pkgconfig/
fi
for i in `seq 0 $((START - 1))`; do
   STAGE="$BUILD_DIR/org.glite.wms/${COMPONENT[$i]}/stage"
   export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$STAGE/$LOCAL_PKGCFG_LIB
done
for i in `seq $START $END`; do
   echo -e "\n*** building component ${COMPONENT[$i]} ***\n"
   
   if [ $7 -ne 0 ]; then
      echo -e "\n*** cleaning up ${COMPONENT[$i]} ***\n"
      cd "$BUILD_DIR/org.glite.wms/${COMPONENT[$i]}"
      make -C build clean 2>/dev/null
      ant clean 2>/dev/null
      python setup.py clean --all 2>/dev/null
      rm -rf rpmbuild RPMS 2>/dev/null
      continue
   fi

   # hack required to integrate not os provided gsoap
   if [ ${COMPONENT[$i]} = "org.glite.wms.wmproxy" ]; then
      if [ $PLATFORM = "sl6" ]; then
         ln -sf "$BUILD_DIR/org.glite.wms/org.glite.wms.wmproxy/src/server/stdsoap2-2_7_16.cpp" \
            "$BUILD_DIR/org.glite.wms/org.glite.wms.wmproxy/src/server/stdsoap2.cpp"
      elif [ $PLATFORM = "sl5" ]; then 
         ln -sf "$BUILD_DIR/org.glite.wms/org.glite.wms.wmproxy/src/server/stdsoap2-2_7_13.cpp" \
            "$BUILD_DIR/org.glite.wms/org.glite.wms.wmproxy/src/server/stdsoap2.cpp"
      fi
   fi

   STAGE="$BUILD_DIR/org.glite.wms/${COMPONENT[$i]}/stage"
   case ${BUILD_TYPE[$i]} in
     "autotools" )
         autotools_build ${COMPONENT[$i]} ${VERSION[$i]} ${AGE[$i]} ${PACKAGE_NAME[$i]} $STAGE
         ;;
     "cmake" )
         cmake_build ${COMPONENT[$i]} ${VERSION[$i]} ${AGE[$i]} ${PACKAGE_NAME[$i]} $STAGE
         ;;
     "ant" )
         ant_build ${COMPONENT[$i]} ${VERSION[$i]} ${AGE[$i]} ${PACKAGE_NAME[$i]} $STAGE
         ;;
     "python" )
         python_build ${COMPONENT[$i]} ${VERSION[$i]} ${AGE[$i]} ${PACKAGE_NAME[$i]} $STAGE
         ;;
     "null" )
         echo "Skipping  ${COMPONENT[$i]}"
         ;;
     "pkg_only" )
         no_build ${COMPONENT[$i]} ${VERSION[$i]} ${AGE[$i]} ${PACKAGE_NAME[$i]}
         ;;
     * )
         echo "Build type not found for ${COMPONENT[$i]}"
         exit
         ;;
   esac

   # the rpm cannot be created from a common stage dir, so why
   # should we have one at all? each 'tmp' dir can be a stage
   export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$STAGE/$LOCAL_PKGCFG_LIB
done

echo -e "\n*** native build completed ***\n"
