C=g++
LFC_TEST_HOME=${PWD}
LFC_HOME=/opt/lcg/include/lfc/
INCL=-I$(LFC_HOME)
SOURCE_FILE=$(LFC_TEST_HOME)
OUTDIR=$(LFC_TEST_HOME)
LFC_LIBS=/opt/lcg/lib/
RANLIB = ranlib
SO_EXT=.so
MKLINK= ln -s
ICLIBPATH=-L$(LFC_LIBS)
THREADLIBS= -lpthread -lcurses -ldl -luuid
CCLIB=$(ICLIBPATH) -llfc $(THREADLIBS)
CFLAGS=$(FLAGS)  -g 
OBJS = lfc_getrep_test.o 

$(OUTDIR)/LFC_GETREP_TEST: $(OBJS) $(LFC_LIBS)/liblfc$(SO_EXT)



	g++ -o $(OUTDIR)/LFC_GET_TEST $(OBJS) $(CCLIB)


lfc_getrep_test.o:lfc_getrep_test.c $(LFC_HOME)/lfc_api.h

	 ${CC} $(CFLAGS)  $(INCL)  -o lfc_getrep_test.o  -c  lfc_getrep_test.c



clean:

	-rm -f *.o 

