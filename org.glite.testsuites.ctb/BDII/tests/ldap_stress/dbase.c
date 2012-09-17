/* Found at http://stsdas.stsci.edu/bps/tdb.c */
/* Modified by Geerd-Dietger.Hoffmann (at) Cern.ch */


#include "dbase.h"


/* So we know how many entries there are */
int enCount = 0;

/* debug mode */
static int debug = 0;

char *
new_record (char *buffer) {
    char *rec = (char *) malloc (SZ_RECORD+1);

    strcpy (rec, buffer);
    return rec;
}

void
drop_record (char *rec) {
    free (rec);
}

int 
cmp_record (char *key, char *rec) {
    for ( ;*key && *rec && *rec == *key; rec ++, key ++)
	;

    if ((*key == ' ' || *key == '\0') && (*rec == ' ' || *rec == '\0'))
	return 0;

    return *key - *rec;
}

dbase * 
read_dbase (char *fname) {
  dbase *db = NULL;
  char *nl = NULL;
  char buffer[SZ_RECORD+1];
  FILE *fd = fopen (fname, "r");

  if (fd == NULL)
    return db;

  while (fgets (buffer, SZ_RECORD, fd)) {
    if ((nl = strchr (buffer, '\n')) == NULL) {
      printf ("Database record truncated:\n%s\n", buffer);
    }
    else{
      *nl = '\0';
    }
    add_dbase(&db, buffer);
    if ( debug) printf ("Added to DB: %s\n", buffer);
  }

  fclose (fd);
  return db;
}

char *
find_dbase (dbase *db, char *key) {
    dbase *ptr = NULL;

    for (ptr = db; ptr != NULL; ptr = ptr->next) {
	if (cmp_record (key, ptr->record) == 0)
	    return ptr->record;
    }

    return NULL;
}

int
del_dbase (dbase **db, char *key) {
    dbase *ptr = NULL;
    dbase *prevptr = NULL;

    for (ptr = *db; ptr != NULL; ptr = ptr->next) {
	if (cmp_record (key, ptr->record) == 0)
	    break;

	prevptr = ptr;
    }

    if (ptr == NULL)
	return 0;

    if (prevptr != NULL) {
	prevptr->next = ptr->next;
    } else {
	*db = ptr->next;
    }

    drop_record (ptr->record);
    free (ptr);

    return -1;
}

int
add_sort_dbase (dbase **db, char *rec) {
    int cmp;
    int status;
    dbase *ptr = NULL;
    dbase *row = NULL;
    dbase *prevptr = NULL;
    dbase *nextptr = NULL;

    row = (dbase *) malloc (sizeof (dbase));
    row->next = NULL;
    row->record = new_record (rec);

    cmp = 1;
    for (ptr = *db; ptr != NULL; ptr = ptr->next) {
	cmp = cmp_record (rec, ptr->record);
	if (cmp <= 0) 
	    break;

	prevptr = ptr;
    }

    if (cmp != 0) {
	nextptr = ptr;
	status = 1;

    } else {
	nextptr = ptr->next;
	status = 0;

	drop_record (ptr->record);
	free (ptr);
    }

    row->next = nextptr;

    if (prevptr != NULL) {
	prevptr->next = row;
    } else {
	*db = row;
    }

    return status;
}

int
add_dbase (dbase **db, char *rec) {
  
  dbase *ptr = NULL;
  dbase *row = NULL;

  row = (dbase *) malloc (sizeof (dbase));
  row->next = NULL;  
  row->record = new_record (rec);

  if (*db == NULL) {
    *db = row;
    enCount = 1;
  }else{
    
 
      for (ptr = *db; ptr->next != NULL; ptr = ptr->next)
	;

    ptr->next = row;
    enCount++;
  }
  
  return 0;
}


void
write_dbase (dbase *db, char *fname) {
      dbase *ptr = NULL;
    FILE *fd = fopen (fname, "w");

    for (ptr = db; ptr != NULL; ptr = ptr->next)
	fprintf (fd, "%s\n", ptr->record);

	fclose (fd);
}

void
drop_dbase (dbase **db) {
    dbase *ptr = NULL;
    dbase *nextptr = NULL;

    for (ptr = *db; ptr != NULL; ptr = nextptr) {
	nextptr = ptr->next;
	drop_record (ptr->record);
	free (ptr);
    }

    *db = NULL;
}

void 
print_dbase(dbase *db){
   dbase *ptr = NULL;
   int i = 0; /* Just so we can print nicely */
   for (ptr = db; ptr != NULL; ptr = ptr->next) {
     printf("[%d/%d]\t%s\n", ++i,enCount, ptr->record);
   }
}

char *
get_random_entry(dbase *db){
   dbase *ptr = NULL;
  int n = 0;
  int counter = 0;
  char *returnVal = NULL;

  n= rand()/(int)(((unsigned)RAND_MAX + 1) / enCount);
  //  printf("-> %d\n",n);
   for (ptr = db; (ptr != NULL) && (++counter < n) ; ptr = ptr->next) {
     ;//     counter++;
   }
    
   returnVal = ptr->record;
  return returnVal;
  
}

dbase * 
init (char *argv, int deb) {
    char *fname = "pass";
    debug = deb;
    

    dbase *db = NULL;

    if (strlen(argv) > 1)
	fname = argv;

    db = read_dbase (fname);
    srand( time(NULL));

    return db;
}
