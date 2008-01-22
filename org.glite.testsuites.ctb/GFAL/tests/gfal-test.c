#include <fcntl.h>
#include <stdio.h>
#include "gfal_api.h"
#define BLKLEN 65536

main(int argc, char **argv)
{
  int fd;
  int i;
  char ibuf[BLKLEN];
  char obuf[BLKLEN];
  int rc;

  if (argc != 2) {
    fprintf (stderr, "usage: %s filename\n", argv[0]);
    exit (1);
  }

  for (i = 0; i < BLKLEN; i++)
    obuf[i] = i;

  printf ("creating file %s\n", argv[1]);
  if ((fd = gfal_open (argv[1], O_WRONLY|O_CREAT, 0644)) < 0) {
    perror ("gfal_open");
    exit (1);
  }
  printf ("open successful, fd = %d\n", fd);

  if ((rc = gfal_write (fd, obuf, BLKLEN)) != BLKLEN) {
    if (rc < 0)
      perror ("gfal_write");
    else
      fprintf (stderr, "gfal_write returns %d\n", rc);
    (void) gfal_close (fd);
    exit (1);
  }
  printf ("write successful\n");

  if ((rc = gfal_close (fd)) < 0) {
    perror ("gfal_close");
    exit (1);
  }
  printf ("close successful\n");

  printf ("reading back %s\n", argv[1]);
  if ((fd = gfal_open (argv[1], O_RDONLY, 0)) < 0) {
    perror ("gfal_open");
    exit (1);
  }
  printf ("open successful, fd = %d\n", fd);

  if ((rc = gfal_read (fd, ibuf, BLKLEN)) != BLKLEN) {
    if (rc < 0)
      perror ("gfal_read");
    else
      fprintf (stderr, "gfal_read returns %d\n", rc);
    (void) gfal_close (fd);
    exit (1);
  }
  printf ("read successful\n");

  if ((rc = gfal_close (fd)) < 0) {
    perror ("gfal_close");
    exit (1);
  }
  printf ("close successful\n");

  for (i = 0; i < BLKLEN; i++) {
    if (ibuf[i] != obuf[i]) {
      fprintf (stderr, "compare failed at offset %d\n", i);
      exit (1);
    }
  }
  printf ("compare successful\n");
  exit (0);
}
