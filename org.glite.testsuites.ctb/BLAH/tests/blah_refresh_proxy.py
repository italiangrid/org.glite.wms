#!/usr/bin/python
 
##########################################################################################
# Script for testing the  BLAH_JOB_SUBMIT and BLAH_JOB_STATUS commands                   #
# It is to be run directly on the machine where BLAH and the LRMS are running            #
##########################################################################################
# It submits a blah_job_submit and a blah_job_refresh_proxy #
# for jobs in 'idle' or 'running status                     #
# command on the subsequent classad jobs from the file      #
# 'classadd_job'                                            #
#                                                           #
# Input value:                                              # 
#              absolute path to the proxy file used         #
#              to submit the job                            # 
#                                                           #
# Returned values:                                          #
#                                                           #
#                 Exit  SAME_OK: Test Passed                #
#                 Exit  SAME_ERROR: Test Failed             #
#                                                           #
#############################################################


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
    res = blah_def.get_results(numLines,fout,fin,20)
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
    print res
    numLines = blah_def.to_int(res.split()[-1])
                                                                                                                   
    #--------------------------------------------------------------------
    #Call the get_results function to loop until some results are found
    #------------------------------------------------------------------- 
    res = blah_def.get_results(numLines,fout,fin,20)
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
            print res
            print "The Job was successfully submitted via the blah_job_submit command, the jobid is: " + jobid + " and the job status is: " + res[res.find("JobStatus") + 14]
            jobstatus =  res[res.find("JobStatus") + 14]
            if ((jobstatus == "1") or (jobstatus == "2")):
              print "about to launch the refresh proxy command" 
    #-------------------------------------------------------------------------------
    #Launch the job_refresh_proxy command and get first result from output
    #-------------------------------------------------------------------------------
              print proxy_file 
              os.write(fin.fileno(), 'blah_job_refresh_proxy 3 ' + jobid + ' ' + proxy_file + '\n')
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
              res = blah_def.get_results(numLines,fout,fin,20)
              numLines = blah_def.to_int(res.split()[-1])
    #------------------------------------------------------------------
    #There was a problem with the refresh proxy command - no results returned
    #------------------------------------------------------------------

              if (numLines == 0):
                 functions.samPrintFAILED()
                 print "Problem with refresh proxy command, no results returned"
                 functions.samNewLine()
                 sys.exit(2)
                                                                                                                      
    #----------------------------------------------------------------------------------
    #Some results were returned, finally get the results of the blah_job_refresh_proxy command
    #----------------------------------------------------------------------------------
              else:
               count = 0
               while ((count < numLines) and (numLines != 0)):
                  res = blah_def.read_line(fout)
                  count += 1
    #---------------------------------------------------------------------------------
    #Get the Jobid returned, in case everything is ok,
    #else check if an error is returned
    #---------------------------------------------------------------------------------
              print "refresh proxy return code"
              print res[2]
              if (res[2] != '0'):
                 functions.samPrintFAILED()
                 print "Problem with refresh proxy command"
                 print res
                 functions.samNewLine()
                 sys.exit(2)
              else:
                 functions.samPrintOK()
                 print "The proxy was refreshed via the blah_job_refresh_proxy command"
                 functions.samNewLine()

            else:
             print "The proxy cannot be refreshed, the job is neither idle or running"
             functions.samNewLine()
             os.write(fin.fileno(), 'quit\n')
             return 0

    os.write(fin.fileno(), 'quit\n')
    return 1 


if __name__ == "__main__":
 #-------------------------------------
 #Get and validate the input parameter
 #-------------------------------------

    try:
      print sys.argv[1]
      proxy_file = sys.argv[1]
    except:
     # print help information and exit:
      functions.samPrintINFO()
      print "Missing parameter: <path_to_the_proxy_file>"
      functions.samNewLine()
      sys.exit(2)

     #----------------
     # Start testing
     #---------------
    
    functions.samPrintINFO()
    print "Start testing the BLAH_JOB_SUBMIT and the BLAH_JOB_STATUS commands ..."
    functions.samNewLine()

    input = open("classadd_job", 'r')
    try:
      print sys.argv[1]
      proxy_file = sys.argv[1]
    except:
     # print help information and exit:
      functions.samPrintINFO()
      print "Missing parameter: <path_to_the_proxy_file>"
      functions.samNewLine()
      sys.exit(2)

    for line in input:
         jobad = line
         functions.samPrintINFO()
         print "Start testing the BLAH_JOB_REFRESH_PROXY command with classad: "
         functions.samNewLine() 
         print jobad
                                                                                                      
         if test_blah_job() :
            functions.samPrintPASSED()
            print "Blah_job_refresh_proxy command completed!"
            functions.samNewLine()
