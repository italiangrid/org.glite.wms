#!/usr/bin/env python
import os

class Config:
    
    SAME_OK          = 10 
    SAME_ERROR       = 50
    SAME_INFO        = 20
    SAME_NOTICE      = 30
    SAME_WARNING     = 40
    SAME_CRITICAL    = 60
    SAME_MAINTENANCE = 100
    
    def __init__(self):
        try:
            self.SAME_OK          = int(os.environ['SAME_OK']) 
            self.SAME_ERROR       = int(os.environ['SAME_ERROR'])
            self.SAME_INFO        = int(os.environ['SAME_INFO'])
            self.SAME_NOTICE      = int(os.environ['SAME_NOTICE'])
            self.SAME_WARNING     = int(os.environ['SAME_WARNING'])
            self.SAME_CRITICAL    = int(os.environ['SAME_CRITICAL'])
            self.SAME_MAINTENANCE = int(os.environ['SAME_MAINTENANCE'])
        except:
            print self.samPrintWARNING("Cannot find SAME_* env variables")
            self.samNewLine()
            
       
    def samPrintERROR(self,text) :
        print '<font color="red"> ERROR: </font>', text, '\n' 
        
    def samPrintOK(self,text) :
        print '<font color="green"> - OK </font>', text , '\n'
        
    def samPrintPASSED(self,text) :
        print '<font color="green"> - PASSED </font>' , text , '\n'
       
    def samPrintFAILED(self,text) :
        print '<font color="red"> - FAILED </font>' , text , '\n'
        
    def samPrintWARNING(self,text) :
        print '<font color="brown"> WARNING: </font>' , text , '\n'
       
    def samPrintINFO(self,text) :
        print '<font color="blue"> INFO: </font>' , text , '\n'
        
    def samNewLine(self,) :
        print '<br>'