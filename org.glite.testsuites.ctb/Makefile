PACKAGE_NAME=lcg-sam-client-sensors-ctb

.PHONY: configure install clean

all: configure

# rpm target

dist:
        @tar --gzip --exclude='*CVS*' -cf build/$(PACKAGE_NAME).src.tgz *

rpm: dist
        @rpmbuild -ta build/$(PACKAGE_NAME).src.tgz

