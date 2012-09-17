SAME_PREF_SE_LIST="$SAME_HOME/sensors/common/prefSE.lst"
SAME_GOOD_SE_FILTER="serviceabbr=SRMv2 type=Production status=Certified tier=0,1 servicestatus=ok servicestatusvo=$SAME_VO"

SAME_PREF_LFC_LIST="$SAME_HOME/sensors/common/prefLFC.lst"
SAME_GOOD_LFC_FILTER="serviceabbr=LFC_C type=Production status=Certified tier=0,1 servicestatus=ok servicestatusvo=$SAME_VO"

LCG_GFAL_BDII_TIMEOUT=20

TIMEOUT_BDII=$LCG_GFAL_BDII_TIMEOUT
TIMEOUT_CONNECT=10
TIMEOUT_SENDRECEIVE=120
TIMEOUT_SRM=180

LFC_CONNTIMEOUT=$TIMEOUT_CONNECT
LFC_CONRETRY=1
LFC_CONRETRYINT=2

# vercmp A B
function vercmp() {
# Compare versions of two packages A and B. 
# Version cat be eg.: 1.56.7test-3alpha
#  0: A and B are the same version
#  1: A is newer than B
#  2: B is newer than A
	# remove non-numerics, '-', '.' and form arrays
	a=( $(echo $1 | tr '[[:alpha:]-.]' ' ') )
	b=( $(echo $2 | tr '[[:alpha:]-.]' ' ') )
	# pad with zeros the shorter one
	while [ "${#a[@]}" -ne "${#b[@]}" ] ; do
		if [ "${#a[@]}" -gt "${#b[@]}" ] ; then
			b=(${b[@]} 0)
		else
			a=(${a[@]} 0)
		fi
	done
	# compare
	for i in $(seq 0 $((${#a[@]} - 1))) ; do
		if [ "${a[$i]}" -gt "${b[$i]}" ] ; then
			return 1
		elif [ "${b[$i]}" -gt "${a[$i]}" ] ; then
			return 2
		fi
	done
	return 0
}

# if installed lcg_util version is greater of or equal to a give one
function lcg_util_ver_ge() {
	if ( lcg-cp --version >/dev/null 2>&1 ) ; then
		lcg_util_curr=$(lcg-cp --version|grep lcg_util)
		lcg_util_curr=${lcg_util_curr/lcg_util-/}
	else
		lcg_util_curr=0.0.0-0
	fi
	
	vercmp $lcg_util_curr $1; rc=$?
	if [ "$rc" -eq "1" ] || [ "$rc" -eq "0" ] ; then
		return 0
	else
		return 1
	fi
}
