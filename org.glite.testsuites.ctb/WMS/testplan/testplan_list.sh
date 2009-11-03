#!/bin/sh
 
exit_code=-1
app_path=`pwd`
celist="N/A"
list_time="N/A"
avg_list_time=0
nlist=5

sanity_check() { # 1 - test_dir  2 - wms.conf
  bad=0
  if [ -z $1 ]; then
    bad=1
  else
    if [ -z $2 ]; then
      bad=1
    fi
  fi
  if [ $bad -eq 1 ]; then
    echo "usage: testplan_list test_dir wms.conf"
    return 1
  fi
  if [ -r "$app_path/testplan_functions" ]; then
    . $app_path/testplan_functions
  else 
    echo "File testplan_functions not found. Quitting.."
    return 1          
  fi 
  return 0
}

sanity_check $1 $2 
if [ $? -eq 0 ]; then
	report_name="$app_path/report.html"
	rm -rf $report_name
	report_header "Test results: list-match"  $report_name
	cd $1
	for jdl in `find . -name \*jdl -maxdepth 1 -type f`; do
    log "processing $jdl"
    i=0
    while [ $i -lt $nlist ]; do
      list_match_single_job $jdl $app_path/$2 
      if [ "x$list_time" != "xERROR" ] && [ "x$list_time" != "xN/A" ]; then 
        avg_list_time=`echo "scale=2 ; $avg_list_time + $list_time"|bc -l`
      else
        avg_list_time=$list_time
        break;
      fi
      i=`expr $i + 1`
    done
    if [ "x$avg_list_time" != "xERROR" ] && [ "x$avg_list_time" != "xN/A" ]; then
      avg_list_time=`echo "scale=2 ; $avg_list_time / $nlist"|bc -l`
    fi
    base="${jdl:0:${#jdl}-4}"
    if [ -r "${base}_list_pre" ]; then
      . "${base}_list_pre" # precondition
    fi
    if [ -r "${base}_list_post" ]; then
      . "${base}_list_post" # postcondition
    fi
        done
        report_footer $report_name
        cd - >/dev/null
fi
