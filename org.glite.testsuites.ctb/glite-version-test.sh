#!/bin/bash

command="/opt/glite/bin/glite-version"
[[ -n $GLITE_LOCATION ]] && command="$GLITE_LOCATION/bin/glite-version"


worker() {
	for f in ${options[*]} ; do
	        echo -n "Testing '$command $f' ..."
	        $command $f > /dev/null
	        if [[ $? -eq 0 ]] ; then 
			echo "OK"
		else
			echo "Failed !"
			ERROR="yes"
		fi
	done
}


# without nodetype
options=( '' '-l'  '-u'  '-v' )
worker

# with node installed
options=( '-v'  '-a'  '-u'  '-v' '-f' )
nodes=`$command -l`
c=$command
for n in ${nodes[*]}; do
	command="$c -n $n"
	worker
done


echo
[[ -n "$ERROR" ]] && echo "Error occured during test!" && exit 1

echo "All tests passed successfully"
echo
exit 0

