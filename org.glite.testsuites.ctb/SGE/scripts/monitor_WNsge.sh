#!/bin/bash

SGE_EXECD=`ps --no-headers -C sge_execd -o %cpu,size|awk 'BEGIN{cpu=0;sz=0} {cpu+=\$1;sz+=\$2} END{print cpu" "sz}'`
TIME=`date -Iminutes`
LA=`sed -e 's/^\([^ ]*\).*\/\([^ ]*\).*/\1 \2/' /proc/loadavg`

echo -e "$TIME\t$LA\t$SGE_EXECD"
