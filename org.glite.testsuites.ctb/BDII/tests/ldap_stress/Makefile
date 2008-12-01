all:
	gcc -Wall -lldap -lpthread -g -o ldapbench ldapbench.c dbase.c

tar: clean
	tar -cvf ldapbench.tar *

clean:
	rm ldapbench
