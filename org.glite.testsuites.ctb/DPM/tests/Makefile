#
# Makefile for creating the DPM-api executable
#
#                   -- Gergely Deberczeni

all: DPM-api

clean:
	rm -f DPM-api.exe

DPM-api: clean DPM-api.c
	rm -rf /tmp/DPM-api.exe
	gcc -I/opt/lcg/include/dpm/ -L/opt/lcg/lib/ -ldpm -ldl DPM-api.c -o DPM-api.exe
