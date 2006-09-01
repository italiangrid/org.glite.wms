DPM_BDII_PORT=2135
BDII_HOST="lxb2017.cern.ch"
BDII_PORT=2170

function failedmessage
{
  echo -n "<b>This DPM test:</b>"; samPrintFAILED
}

function passedmessage
{
  echo -n "<b>This DPM test:</b>"; samPrintPASSED
}

function warningmessage
{
  echo -n "<b>This DPM test:</b>"; samPrintWARNING
}
function infomessage
{
  echo -n "<b>This DPM test:</b>"; samPrintINFO
}

