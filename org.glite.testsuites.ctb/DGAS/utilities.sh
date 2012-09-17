
### Check if the environment variales used in the test exist
### Parameter: the list of the envs to check
function check_env
{
  samPrintINFO ; echo " Check the environment... " ; samNewLine

  ret=0

  for env in $1 ; do
    if [ -z $(eval echo "$"$env) ]; then
      samPrintFAILED ; echo " >>> $env is not define! <<<" ; samNewLine
      ret=1
    fi
  done

  if [ $ret -eq 1 ] ; then
    samPrintWARNING ; echo " Check the configuration file." ; samNewLine
    exit $SAME_ERROR
  fi
}

### Check if the commands used in the test exist
### Parameter: the list of the commands to check
function check_command
{
  samPrintINFO ; echo " Check the commands... " ; samNewLine

  ret=0

  for command in $1 ; do
    if [ ! -x $command ]; then
      samPrintFAILED ; echo " >>> $command not exists! <<<" ; samNewLine
      ret=1
    fi
  done

  if [ $ret -eq 1 ] ; then
    samPrintWARNING ; echo " Check the installation." ; samNewLine
    exit $SAME_ERROR
  fi
}

