options2short = { \
"all": ["all","",""] ,\
"autm-delegation": ["autm-delegation","a",""] ,\
"cancel-input": ["input","i","filepath"] ,\
"cancel-output": ["output","o","filepath"] ,\
"collection": ["collection","c","dirpath"] ,\
"config":["config","c","configfile"] ,\
"debug": ["debug","",""] ,\
"delegationid": ["delegationid","d","idstring"] ,\
"deleg-output": ["output","o","filepath"] ,\
"endpoint": ["endpoint","e","serviceURL"] ,\
"filename": ["filename","f","filename"] ,\
"get": ["get","",""] ,\
"help": ["help","",""] ,\
"jobout-dir": ["dir","","directorypath"] ,\
"jobout-input": ["input","i","filepath"] ,\
"list-only": ["list-only","",""] ,\
"logfile": ["logfile","","filepath"] ,\
"lrms": ["lrms","","lrmstype"] ,\
"match-output": ["output","o","filepath"] ,\
"nodes-resources": ["nodes-resources","","ceid"] ,\
"nodisplay": ["nodisplay","",""] ,\
"noint": ["noint","",""] ,\
"nolisten": ["nolisten","",""] ,\
"nomsg": ["nomsg","",""] ,\
"perusal-dir": ["dir","","directorypath"] ,\
"perusal-output": ["output","o","filepath"] ,\
"proto": ["proto","","protocol"] ,\
"rank": ["rank","",""] ,\
"register-only": ["register-only","",""] ,\
"resource": ["resource","r", "ceid"] ,\
"set": ["set","",""] ,\
"start": ["start","","jobid"] ,\
"submit-input": ["input","i","filepath"] ,\
"submit-output": ["output","o","filepath"] ,\
"to": ["to ","","[MM:DD:]hh:mm[:[CC]YY]"] ,\
"transfer-files": ["transfer-files","",""] ,\
"unset": ["unset","",""] ,\
"valid": ["valid","v","hh:mm"] ,\
"version": ["version","",""] ,\
"vo": ["vo","","voname"] ,\
}

common_options=[ "version", "help", "config", "vo", "debug", "logfile", "noint"]

commands2options={
"glite-wms-job-delegate-proxy":["delegationid", "autm-delegation", "endpoint", "deleg-output"], \
"glite-wms-job-submit":["submit-input", "resource","nodes-resources",  "nolisten", "nomsg","lrms", "to", "valid", "register-only", \
"transfer-files", "proto", "start", "submit-output", "collection"], \
"glite-wms-job-cancel":["cancel-input","cancel-output"], \
"glite-wms-job-list-match":["delegationid", "autm-delegation", "endpoint", "rank","match-output"], \
"glite-wms-job-output":["jobout-input", "jobout-dir", "list-only"], \
"glite-wms-job-perusal": ["set", "get", "unset", "filename", "all","perusal-dir",  "perusal-output", "nodisplay"] \
}
