# DPM sensor configuration file
#  
#

DPM_BDII_PORT=2135
BDII_HOST="lxb2017.cern.ch"
BDII_PORT=2170

function thisTest 
{
  echo -n "<b>This DPM test:</b>"
}

function failedmessage
{
  thisTest ; samPrintFAILED
}

function passedmessage
{
  thisTest; samPrintPASSED
}

function warningmessage
{
  thisTest; samPrintWARNING
}
function infomessage
{
  thisTest; samPrintINFO
}

