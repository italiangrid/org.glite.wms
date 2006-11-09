PACKAGE_NAME=lcg-sam-client-sensors-ctb
####################################################################
# Distribution Makefile
####################################################################

.PHONY: configure install clean

all: configure

####################################################################
# Prepare
####################################################################

prepare:
	rm -f *~ functions/*~ scripts/*~ examples/*~ 
	@mkdir -p build/doc/html/web
	@mkdir -p build/doc/man

####################################################################
# Configure
####################################################################

configure: 
	@echo "No configuration required, use either 'make install' or 'make rpm'."

####################################################################
# Compile
####################################################################

compile: 
	@echo "No compiling required, use either 'make install' or 'make rpm'."

####################################################################
# Install
####################################################################

install: 
	@echo installing ...
	mkdir -p $(prefix)/
	cp -R DPM $(prefix)/

####################################################################
# Documentation
####################################################################

doc: man html

man: prepare

html: prepare

web: html

####################################################################
# Install Doc
####################################################################

install-doc: doc
	@echo installing  docs...

####################################################################
# Build Distribution
####################################################################

dist: prepare 
	@tar --gzip --exclude='*CVS*' -cf build/$(PACKAGE_NAME).src.tgz *

rpm: dist
	@rpmbuild -ta build/$(PACKAGE_NAME).src.tgz 

clean::
	rm -f *~ test/*~ etc/*~ doc/*~ src/*~ $(PACKAGE_NAME).src.tgz 
	rm -rf build 

