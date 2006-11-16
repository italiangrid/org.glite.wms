#!/usr/bin/python

#------------------------------------------------------------
#SAM functions definitions
#-----------------------------------------------------------

def samPrintERROR(): 
      print  "<font color=red> Error: </font>"

def samPrintOK():
      print "<font color=green> - OK </font>"

def samPrintPASSED():
      print "<font color=green> - PASSED </font>"

def samPrintFAILED():
      print "<font color=red> - FAILED </font>"

def samPrintWARNING():
      print "<font color=brown> WARNING: </font>:"

def samPrintINFO():
      print "<font color=blue> INFO: </font>:"

def samNewLine():
      print "<br>"

if __name__ == "__main__":
     samPrintERROR()
