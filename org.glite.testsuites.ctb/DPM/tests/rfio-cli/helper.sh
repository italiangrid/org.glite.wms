BOOTUP=color
RES_COL=80
RES_COL_1=30
MOVE_TO_COL="echo -en \\033[${RES_COL}G"
MOVE_TO_COL_1="echo -en \\033[${RES_COL_1}G"
SETCOLOR_SUCCESS="echo -en \\033[1;32m"
SETCOLOR_FAILURE="echo -en \\033[1;31m"
SETCOLOR_WARNING="echo -en \\033[1;33m"
SETCOLOR_NORMAL="echo -en \\033[0;39m"

echo_success() {
  [ "$BOOTUP" = "color" ] && $MOVE_TO_COL
  echo -n "[  "
  [ "$BOOTUP" = "color" ] && $SETCOLOR_SUCCESS
  echo -n $"OK"
  [ "$BOOTUP" = "color" ] && $SETCOLOR_NORMAL
  echo -n "  ]"
  echo -ne "\r"
  return 0
}

echo_failure() {
  [ "$BOOTUP" = "color" ] && $MOVE_TO_COL
  echo -n "["
  [ "$BOOTUP" = "color" ] && $SETCOLOR_FAILURE
  echo -n $"FAILED"
  [ "$BOOTUP" = "color" ] && $SETCOLOR_NORMAL
  echo -n "]"
  echo -ne "\r"
  return 1
}

echo_passed() {
  [ "$BOOTUP" = "color" ] && $MOVE_TO_COL
  echo -n "["
  [ "$BOOTUP" = "color" ] && $SETCOLOR_WARNING
  echo -n $"PASSED"
  [ "$BOOTUP" = "color" ] && $SETCOLOR_NORMAL
  echo -n "]"
  echo -ne "\r"
  return 1
}

echo_warning() {
  [ "$BOOTUP" = "color" ] && $MOVE_TO_COL
  echo -n "["
  [ "$BOOTUP" = "color" ] && $SETCOLOR_WARNING
  echo -n $"WARNING"
  [ "$BOOTUP" = "color" ] && $SETCOLOR_NORMAL
  echo -n "]"
  echo -ne "\r"
  return 1
}

progress()
{
        $MOVE_TO_COL_1
	local width=30
	local max=$1 curr=$2 pos=$width perc=100
	if [ $curr -lt $max ]; then
		pos=$((curr*width/max))
		perc=$((curr*100/max))
	fi
	local str="[$(printf "%${pos}s" \
		| sed 's/ /#/g')$(printf "%$((width-$pos))s" \
		| sed 's/ /-/g')]"
	[ -z "$PB_NOPERC" ] && str="$str $perc%"
	[ -z "$PB_NOITEM" ] && str="$str ($i/$j)"
	echo $str
        echo -en "\r"
	[ $perc -eq 100 ]
}
