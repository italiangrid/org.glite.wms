#!/usr/bin/python
 
##########################################################################################
# Script for testing the  BLAH_JOB_SUBMIT and BLAH_JOB_STATUS commands works             #
# It is to be ran directly on the machine where BLAH and the LRMS are running            #
##########################################################################################
# It submits a blah_job_submit and a blah_job_status    #
# command on the subsequent classad jobs from the file  #
# 'classadd_job'                                        #
#                                                       #
# Returned values:                                      #
#                                                       #
#                 Exit  SAME_OK: Test Passed            #
#                 Exit  SAME_ERROR: Test Failed         #
#                 Exit  2: Wrong Input                  #
#                                                       #
#########################################################


import os, sys, time, string, functions, blah_def

 
def test_blah_job():
    #-------------------------------------------------
    #Start BLAH
    #------------------------------------------------- 

    (fin, fout, ferr) = os.popen3("/opt/glite/bin/blahpd")

    #-------------------------------------------------
    #Read BLAH Heading line
    #-------------------------------------------------
    blah_def.read_line(fout)

    #------------------------------------------------
    #Launch the BLAH_JOB_SUBMIT command
    #------------------------------------------------
    submitCmd = "BLAH_JOB_SUBMIT 1 " + jobad + "\n"
    os.write(fin.fileno(), submitCmd)
    blah_def.read_line(fout)

    #-------------------------------------------------
    #Get results 
    #------------------------------------------------
    
    os.write(fin.fileno(), 'results\n')
    res = blah_def.read_line(fout)
    numLines = blah_def.to_int(res.split()[-1])
    
    #-------------------------------------------------------------------
    #Call the get_results function to loop until some results are found
    #------------------------------------------------------------------- 
    res = blah_def.get_results(numLines,fout,fin)
    numLines = blah_def.to_int(res.split()[-1])

    #------------------------------------------------------------------
    #There was a problem with the submit command - no results returned
    #------------------------------------------------------------------
    if (numLines == 0):
            functions.samPrintFAILED()
            print "Problem with submit command, no results returned"
            functions.samNewLine()
            sys.exit(2)

    #-----------------------------------------------------------------------------------
    #Some results were returned, finally get the results of the blah_job_submit command  
    #-----------------------------------------------------------------------------------
    else:
            count = 0
            while ((count < numLines) and (numLines != 0)):
                res = blah_def.read_line(fout) 
                count += 1

    #---------------------------------------------------------------------------------
    #Get the Jobid returned, in case everything is ok,
    #else check if an error is returned  
    #---------------------------------------------------------------------------------  
    returnCode = res[2]
    if (returnCode != '0'):
            functions.samPrintFAILED()
            print "blah_job_submit had a problem: " + returnCode
            functions.samNewLine()
            sys.exit(2)
    jobid = res.split()[-1]   

    #-------------------------------------------------------------------------------
    #Launch the jobstatus command and get first result from output
    #-------------------------------------------------------------------------------
    os.write(fin.fileno(), 'blah_job_status 2 ' + jobid + '\n')
    blah_def.read_line(fout)

    #-------------------------------------------------------------------------------
    #Do the same as above, loop until some results are found
    #-------------------------------------------------------------------------------
    os.write(fin.fileno(), 'results\n')
    res = blah_def.read_line(fout)
    numLines = blah_def.to_int(res.split()[-1])
                                                                                                                   
    #--------------------------------------------------------------------
    #Call the get_results function to loop until some results are found
    #------------------------------------------------------------------- 
    res = blah_def.get_results(numLines,fout,fin)
    numLines = blah_def.to_int(res.split()[-1])
    
    #------------------------------------------------------------------
    #There was a problem with the status command - no results returned
    #------------------------------------------------------------------
    if (numLines == 0):
            functions.samPrintFAILED()
            print "Problem with status command, no results returned"
            functions.samNewLine()
            sys.exit(2)
    
    #----------------------------------------------------------------------------------
    #Some results were returned, finally get the results of the blah_job_status command
    #----------------------------------------------------------------------------------
    else:
            count = 0
            while ((count < numLines) and (numLines != 0)):
                res = blah_def.read_line(fout)
                count += 1

    #----------------------------------------------------------------------------------------
    #Check the error code if it is not zero, it means there was a problem otherwise it is ok
    #with the job_status
    #----------------------------------------------------------------------------------------
    returnCode = res[2]
    if (returnCode != '0'):
            functions.samPrintFAILED()
            print "blah_job_status had a problem: " + returnCode
            functions.samNewLine()
            sys.exit(2)
    else :
            functions.samPrintOK()
            print "The Job was successfully submitted via the blah_job_submit command, the jobid is: " + jobid + " and the job status is: " + res[5]
            functions.samNewLine() 
    os.write(fin.fileno(), 'quit\n')
    return 1 


if __name__ == "__main__":
     #----------------
     # Start testing
     #---------------
    
    functions.samPrintINFO()
    print "Start testing the BLAH_JOB_SUBMIT and the BLAH_JOB_STATUS commands ..."
    functions.samNewLine()

    input = open("classadd_job", 'r')
    for line in input:
         jobad = line
         functions.samPrintINFO()
         print "Start testing the BLAH_JOB_SUBMIT and the BLAH_JOB_STATUS commands with classad: "
         functions.samNewLine() 
         print jobad
                                                                                                      
         if test_blah_job() :
            functions.samPrintPASSED()
            print "Blah_job_submit and Blah_job_status completed!"
            functions.samNewLine()
