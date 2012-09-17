all:
	gcc -Wall -DLDAP_DEPRECATED -lldap_r -lpthread -g -o ldapbench ldapbench.c dbase.c

tar: clean
	tar -cvf ldapbench.tar *

clean:
	rm ldapbench
