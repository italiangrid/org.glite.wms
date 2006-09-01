#
# This file is for common BASH functions used by the sensors.
# It's aim is to help maintening a coherent test output layout.
# 
#                             -- gdebrecz

function samPrintERROR {
 echo -n "<font color="red"> Error: </font>"
}

function samPrintOK {
 echo "<font color="green"> - OK </font>"
}

function samPrintPASSED {
 echo "<font color="green"> - PASSED </font>"
}

function samPrintFAILED {
 echo "<font color="red"> - FAILED </font>"
}

function samPrintWARNING {
 echo -n "<font color="brown"> WARNING: </font>:"
}

function samPrintINFO {
 echo -n "<font color="blue"> INFO: </font>:"
}

function samNewLine {
 echo "<br>"
}
