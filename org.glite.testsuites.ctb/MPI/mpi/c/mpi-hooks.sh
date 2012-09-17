#!/bin/sh

#
# This function will be called before the MPI executable is started.
# You can, for example, compile the executable itself.
#
pre_run_hook () {

  # Compile the program.
  echo "Compiling ${I2G_MPI_APPLICATION}"

  # Actually compile the program.
  cmd="mpicc ${MPI_MPICC_OPTS} -o ${I2G_MPI_APPLICATION} ${I2G_MPI_APPLICATION}.c"
  echo $cmd
  $cmd
  if [ ! $? -eq 0 ]; then
    echo "Error compiling program.  Exiting..."
    exit 1
  fi

  # Everything's OK.
  echo "Successfully compiled ${I2G_MPI_APPLICATION}"

  return 0
}

#
# This function will be called before the MPI executable is finished.
# A typical case for this is to upload the results to a storage element.
#
post_run_hook () {

  echo "Executing post hook."
  echo "Finished the post hook."

  return 0
}
