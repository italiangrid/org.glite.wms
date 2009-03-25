# CONFIGURATION VALUES:
"""
DEBUG values:
0 - no debug
1 - stream debug (all toghether)
2 - step debug
"""
RASKMAN_DEBUG = 0
# Detemrine which documentation to be generated:
GENERATE_MAN=True #MAN PAGES
GENERATE_TXT=True #LATEC
GENERATE_WKI=True #WIKI PAGES
GENERATE_PLT=False #Plain Text PAGES
#Output for generated pages PATH:
MAN_OUTPUT=""
TXT_OUTPUT="../../org.egee.jra1.deliverables/users-guide/WMPROXY/"
WKI_OUTPUT=""
PLT_OUTPUT="../config/" + "glite_wmsui_cmd_help.conf"
# OPTIONS and COMMANDS description files:
options2descriptionFile  = "options_description.txt"
commands2descriptionFile = "commands_description.txt"


options2short = { \
"all": ["all","",""] ,\
"autm-delegation": ["autm-delegation","a",""] ,\
"cancel-input": ["input","i","filepath"] ,\
"info-input": ["input","i","filepath"] ,\
"cancel-output": ["output","o","filepath"] ,\
"collection": ["collection","","dirpath"] ,\
"config":["config","c","configfile"] ,\
"dag": ["dag","","dirpath"] ,\
"debug": ["debug","",""] ,\
"default-jdl": ["default-jdl","","filepath"] ,\
"delegationid": ["delegationid","d","idstring"] ,\
"deleg-output": ["output","o","filepath"] ,\
"endpoint": ["endpoint","e","serviceURL"] ,\
"filename": ["filename","f","filename"] ,\
"get": ["get","",""] ,\
"help": ["help","",""] ,\
"info-output" : ["output","o","filename"],\
"info-delegationid": ["delegationid","d","idstring"] ,\
"jobout-dir": ["dir","","directorypath"] ,\
"jobout-input": ["input","i","filepath"] ,\
"jdl":["jdl","","jobid"], \
"jdl-original":["jdl-original","j","jobid"], \
"list-only": ["list-only","",""] ,\
"logfile": ["logfile","","filepath"] ,\
"lrms": ["lrms","","lrmstype"] ,\
"match-output": ["output","o","filepath"] ,\
"nodes-resource": ["nodes-resource","","ceid"] ,\
"nodisplay": ["nodisplay","",""] ,\
"noint": ["noint","",""] ,\
"nolisten": ["nolisten","",""] ,\
"nomsg": ["nomsg","",""] ,\
"perusal-dir": ["dir","","directorypath"] ,\
"perusal-output": ["output","o","filepath"] ,\
"proto": ["proto","","protocol"] ,\
"proto-submit": ["proto","","protocol"] ,\
"proxy": ["proxy","p","jobid"] ,\
"rank": ["rank","",""] ,\
"register-only": ["register-only","",""] ,\
"resource": ["resource","r", "ceid"] ,\
"set": ["set","",""] ,\
"start": ["start","","jobid"] ,\
"submit-input": ["input","i","filepath"] ,\
"perusal-inputfile": ["input-file","","filepath"] ,\
"submit-output": ["output","o","filepath"] ,\
"to": ["to ","","[MM:DD:]hh:mm[:[CC]YY]"] ,\
"transfer-files": ["transfer-files","",""] ,\
"unset": ["unset","",""] ,\
"valid": ["valid","v","hh:mm"] ,\
"version": ["version","",""] ,\
"vo": ["vo","","voname"] ,\
"all_python": ["all","",""] ,\
"input_python": ["input","i","filepath"] ,\
"output_python": ["output","o","filepath"] ,\
"config_python":["config","c","configfile"] ,\
"debug_python": ["debug","",""] ,\
"help_python": ["help","",""] ,\
"dir_python": ["dir","","directorypath"] ,\
"logfile_python": ["logfile","","filepath"] ,\
"lrms_python": ["lrms","","lrmstype"] ,\
"output_python": ["output","o","filepath"] ,\
"nodes-resource_python": ["nodes-resource","","ceid"] ,\
"noint_python": ["noint","",""] ,\
"nogui_python": ["nogui","",""] ,\
"nolisten_python": ["nolisten","",""] ,\
"nomsg_python": ["nomsg","",""] ,\
"nonodes_python": ["nonodes","",""] ,\
"rank_python": ["rank","",""] ,\
"resource_python": ["resource","r", "ceid"] ,\
"from_python": ["from","","[MM:DD:]hh:mm[:[CC]YY]"] ,\
"to_python": ["to","","[MM:DD:]hh:mm[:[CC]YY]"] ,\
"user-tag_python": ["user-tag","","<tag name>=<tag value>"] ,\
"status_python": ["status","s","<status code>"] ,\
"event_python": ["event","","<event code>"] ,\
"excludeS_python": ["exclude","e","<status code>"] ,\
"excludeL_python": ["exclude","e","<event code>"] ,\
"submit-to_python": ["to","","[MM:DD:]hh:mm[:[CC]YY]"] ,\
"valid_python": ["valid","v","hh:mm"] ,\
"version_python": ["version","",""] ,\
"verbosity_python": ["verbosity","v","level"] ,\
"vo_python": ["vo","","voname"] ,\
"cs_python": ["cs","","chkptStep"] ,\
"port_python": ["port","p","<port number>"] ,\
}

common_options=[ "version", "help", "config", "vo", "debug", "logfile", "noint","version_python", "help_python", "config_python", "debug_python", "logfile_python", "noint_python"]

commands2options={
"glite-wms-job-delegate-proxy":["delegationid", "autm-delegation", "endpoint", "deleg-output"], \
"glite-wms-job-submit":["delegationid", "autm-delegation","submit-input", "resource","nodes-resource",  "nolisten", "nomsg","lrms", "to", "valid", "register-only", \
   "transfer-files", "proto-submit", "start", "submit-output", "collection","dag","default-jdl"], \
"glite-wms-job-cancel":["cancel-input","cancel-output"], \
"glite-wms-job-list-match":["delegationid", "autm-delegation", "endpoint", "rank","match-output"], \
"glite-wms-job-output":["jobout-input", "jobout-dir", "proto", "list-only"], \
"glite-wms-job-perusal": ["perusal-inputfile","set", "get", "unset", "filename",\
   "all", "perusal-dir", "proto", "perusal-output", "nodisplay"],\
"glite-wms-job-info":["info-input","info-delegationid","proxy","jdl","jdl-original","endpoint","info-output"],\
"glite-wms-job-logging-info":["input_python","output_python","verbosity_python","from_python","to_python",\
"user-tag_python","event_python", "excludeL_python"], \
"glite-wms-job-status":["input_python","output_python","all_python","verbosity_python","from_python","to_python",\
"user-tag_python","status_python","excludeS_python","nonodes_python"], \
}
