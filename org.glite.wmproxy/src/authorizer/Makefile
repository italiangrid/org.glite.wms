C=g++
COFLAGS=-O2
CWFLAGS=
ROOT=../../..
REPOSITORY=$(ROOT)/repository
STAGE=$(ROOT)/stage
OBJFLAGS    = -g -Wall

CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS)


LIB_DEST=../lib


# gaclmanager opts
#--------------------
BOOST_LIBS=$(REPOSITORY)/boost/1.29.1/rhel30_gcc32/lib/release/libboost_fs.a \
	$(REPOSITORY)/boost/1.29.1/rhel30_gcc32/lib/release/libboost_thread.a \
	$(REPOSITORY)/boost/1.29.1/rhel30_gcc32/lib/release/libboost_regex.a

CLASSAD_LIBS=$(REPOSITORY)/classads/0.9.6/rhel30_gcc32/lib/libclassad.a

LIB_NAME_MGR=libgacl-manager.so

SRC_MGR=gaclmanager.cpp  gaclexception.cpp

INCLUDE_MGR=-I./ -I/usr/include/openssl/ -I/usr/kerberos/include \
	-I$(ROOT)/org.glite.wms-utils.exception/interface \
	-I$(ROOT)/org.gridsite.core/interface/

LIBS_MGR=-lcurl -lz -lssl$(FLAVOR_EXT) -lcrypto_gcc32pthr -lxml2 -lm -lglite_wmsutils_exception \
	-lgridsite  -lmcnab \
	-L$(STAGE)/lib \
	-L/usr/lib  -L$(REPOSITORY)/globus/2.4.3-VDT-1.2.2/rhel30_gcc32/lib


# gacladmin opts
#--------------------
SRC_ADMIN=gacladministator.cpp

OBJ_ADMIN=wms-gacl-admin

INCLUDE_ADMIN=-I./ $(INCLUDE_MGR)

BOOST_LIBS=$(REPOSITORY)/boost/1.29.1/rhel30_gcc32/lib/release/libboost_fs.a \
	$(REPOSITORY)/boost/1.29.1/rhel30_gcc32/lib/release/libboost_thread.a \
	$(REPOSITORY)/boost/1.29.1/rhel30_gcc32/lib/release/libboost_regex.a

CLASSAD_LIBS=$(REPOSITORY)/classads/0.9.6/rhel30_gcc32/lib/libclassad.a

LIBS_ADMIN= -lgacl-manager -lglite_wms_util \
	$(LIBS_MGR) -L ../lib \
     	$(BOOST_LIBS) \
     	$(CLASSAD_LIBS) \
	-L./


all: manager admin

manager:
	$(C)  $(CFLAGS) -shared \
	-o $(LIB_NAME_MGR) $(SRC_MGR) \
	$(INCLUDE_MGR) $(LIBS_MGR)



admin:
	$(C) $(OBJFLAGS) \
	-o $(OBJ_ADMIN)\
	$(SRC_ADMIN) \
	$(INCLUDE_ADMIN) $(LIBS_ADMIN)





