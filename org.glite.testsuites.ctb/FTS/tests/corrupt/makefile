#
# Makefile for corrupt
#
# Alejandro Álvarez Ayllón
#

CXX=gcc
INCLUDE=-I /opt/lcg/include/lcgdm -I /opt/lcg/include/dpm
LIBS=-L/opt/lcg/lib -ldpm -ldl
OBJS=corrupt.o
TARGET=corrupt

all:$(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(LIBS) $^

%.o: %.c
	$(CXX) $(INCLUDE) -c -o $@ $<

