#!/bin/sh

# Configurable parameters

# Defaults values
USER=1
END=1
START=0

# Help 
help()
{
  echo -e "Usage: $0 [option]\n"
  echo -e " -h \t\t\t This help;"
  echo -e " -s Start \t\t Start ananlisys from token Start (default $START);"
  echo -e " -e End \t\t End analisys to token End (default $END);"
  echo -e " -u #user \t\t Number of users (default $USER);" 
  exit 0
}



while getopts "hs:e:u:" arg
do
  case "$arg" in
	h) help ;;
  e) END=$OPTARG ;;
  s) START=$OPTARG ;;
  u) USER=$OPTARG;;
  *) echo "Parameter not valid"; exit 1 ;;
  esac
done


Num=(0 0 0 0)

count() 
{
	name=$1
	i=0
	for file in done notdone abort canc ; do
		if [[ -f ${name}.${file} ]]; then
			Num[$i]=$(( ${Num[$i]} + `wc -l ${name}.${file} 2> /dev/null | gawk -F" " '{print $1}'`  ))
		fi
		i=$(( $i + 1 ))
	done 	
}

for (( u=0 ; $u < $USER ; u = $(($u + 1)) )); do
	for (( p=$START ; $p <= $END ; p = $(($p + 1)) )); do
		rm -f USER${u}_${p}.out
		if [[ -f USER${u}_${p}.notdone ]]; then
     	glite-wms-job-status -v 2  -o USER${u}_${p}.out --noint -i USER${u}_${p}.notdone
			# remove jobid file, it shoul be recreate by parser
      rm USER${u}_${p}.notdone
		elif [[ -f USER${u}_${p}.jobid ]] ; then  # first analysis
			glite-wms-job-status -v 2  -o USER${u}_${p}.out --noint -i USER${u}_${p}.jobid
			# save old jobid
			mv USER${u}_${p}.jobid USER${u}_${p}.jobid.old
		fi
		if [[ -f USER${u}_${p}.out ]] ; then
			python `dirname $0`/parse.py -f USER${u}_${p}.out >> Output_Part${p}.txt 
		fi
			count USER${u}_${p}
	done
done

printf '\033[1;31m Results:\033[0m\n'
printf 'DONE   : \t %6d \n' ${Num[0]}
printf 'NOTDone: \t \033[1;32m%6d \033[0m\n' ${Num[1]}
printf 'Aborted: \t %6d \n' ${Num[2]}

if [[ ${Num[3]} -ne 0 ]] ; then
	printf 'Cancelled: \t %6d \n' ${Num[3]}
fi
echo "------------------------"
printf 'Total  : \t %6d \n' $(( ${Num[0]} + ${Num[1]} + ${Num[2]} + ${Num[3]}))


