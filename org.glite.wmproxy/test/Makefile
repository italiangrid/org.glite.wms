CPP=g++

SRC=../src/server
LDFLAGS=-L../src/server -lwmpdaemon
INCLUDE=-I../src/server

dir:lib
	@$(CPP)  -Wall  -DHAVE_CONFIG_H -DALLOW_COMMAND_LINE_OVERRIDE $(INCLUDE) $(CFLAGS) -o just-dir adjustdir.c  $(LDFLAGS)
	@echo DONE
lib:
	@$(CPP)  -Wall  -DHAVE_CONFIG_H $(INCLUDE) -shared -o libwmpdaemon.so  $(SRC)/wmpdaemon.cpp
