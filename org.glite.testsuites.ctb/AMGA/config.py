#!/usr/bin/env python
import os

class ExitCodes:
    
    SAME_OK          = 10 
    SAME_ERROR       = 50
    SAME_INFO        = 20
    SAME_NOTICE      = 30
    SAME_WARNING     = 40
    SAME_CRITICAL    = 60
    SAME_MAINTENANCE = 100
    
    def __init__(self):
        try:
            self.SAME_OK          = os.environ['SAME_OK'] 
            self.SAME_ERROR       = os.environ['SAME_ERROR']
            self.SAME_INFO        = os.environ['SAME_INFO']
            self.SAME_NOTICE      = os.environ['SAME_NOTICE']
            self.SAME_WARNING     = os.environ['SAME_WARNING']
            self.SAME_CRITICAL    = os.environ['SAME_CRITICAL']
            self.SAME_MAINTENANCE = os.environ['SAME_MAINTENANCE']
        except:
            print samPrintWARNING("Cannot find SAME_* env variables")
            samNewLine()
            
       
def samPrintERROR(text) :
    print '<font color="red"> ERROR: </font>', text, '\n' 
    
def samPrintOK(text) :
    print '<font color="green"> - OK </font>', text , '\n'
    
def samPrintPASSED(text) :
    print '<font color="green"> - PASSED </font>' , text , '\n'
   
def samPrintFAILED(text) :
    print '<font color="red"> - FAILED </font>' , text , '\n'
    
def samPrintWARNING(text) :
    print '<font color="brown"> WARNING: </font>' , text , '\n'
   
def samPrintINFO(text) :
    print '<font color="blue"> INFO: </font>' , text , '\n'
    
def samNewLine() :
    print '<br>'