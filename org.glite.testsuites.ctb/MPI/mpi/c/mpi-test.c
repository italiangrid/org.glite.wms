/*  hello.c
 *
 *  Simple "Hello World" program in MPI.
 *
 */

#include "mpi.h"
#include <stdio.h>
int main(int argc, char *argv[]) {

  int numprocs;  /* Number of processors */
  int procnum;   /* Processor number */

  /* Initialize MPI */
  MPI_Init(&argc, &argv);

  /* Find this processor number */
  MPI_Comm_rank(MPI_COMM_WORLD, &procnum);

  /* Find the number of processors */
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  printf ("Hello world! from processor %d out of %d\n", procnum, numprocs);

  /* Shut down MPI */
  MPI_Finalize();
  return 0;
}
