/* Found at http://stsdas.stsci.edu/bps/tdb.c */
/* Modified by Geerd-Dietger.Hoffmann (at) Cern.ch */

/* The h file */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define  SZ_RECORD	256	


struct dbase_rec {
    struct dbase_rec	*next;
    char		*record;
};

typedef struct dbase_rec dbase;


void	drop_record (char *rec);
int 	cmp_record (char *key, char *rec);
dbase  *read_dbase (char *fname);
char   *find_dbase (dbase *db, char *key);
int	del_dbase (dbase **db, char *key);
int	add_dbase (dbase **db, char *rec);
int	add_sort_dbase (dbase **db, char *rec);
void	write_dbase (dbase *db, char *fname);
void	drop_dbase (dbase **db);
void    print_dbase(dbase *db);
char   *get_random_entry(dbase *db);
dbase	*init (char *argv, int deb);
