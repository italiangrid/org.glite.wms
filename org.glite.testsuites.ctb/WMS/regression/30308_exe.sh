#!/bin/sh

# The first parameter is the binary to be executed

EXE=$1

# The second parameter is the number of CPU's to be reserved for parallel execution

CPU_NEEDED=$2

# prints the name of the master node

echo "Master node is: $HOSTNAME"

echo "Is should run on the following nodes:"

cat $PBS_NODEFILE

echo "*************************************"

# prints the working directory on the master node

echo "Current working directory is: $PWD"

# prints the list of files in the working directory

echo "List files on the working directory:"

ls -alR `pwd`

# execute the parallel job with mpirun

echo "*********************************"

chmod 777 $EXE

echo "Now the following executable $EXE should be run."

mpirun -np $CPU_NEEDED -machinefile $PBS_NODEFILE `pwd`/$EXE >& executable.out


echo "*********************************" 
