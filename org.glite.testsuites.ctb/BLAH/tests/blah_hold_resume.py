#!/usr/bin/python
 
##########################################################################################
# Script for testing the  BLAH_JOB_HOLD and BLAH_JOB_RESUME commands                     #
# It is to be run directly on the machine where BLAH and the LRMS are running            #
##########################################################################################
#                                                       #
# Returned values:                                      #
#                                                       #
#                 Exit  SAME_OK: Test Passed            #
#                 Exit  SAME_ERROR: Test Failed         #
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
    res = blah_def.get_results(numLines,fout,fin,40)
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
    res = blah_def.get_results(numLines,fout,fin,40)
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
            jobstatus = blah_def.to_int(res[5])
            if ((jobstatus == 1) or (jobstatus == 2)):
    #-------------------------------------------------------------------------------
    #Launch the job_hold command and get first result from output
    #-------------------------------------------------------------------------------
              os.write(fin.fileno(), 'blah_job_hold 3 ' + jobid + '\n')
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
    #There was a problem with the hold command - no results returned
    #------------------------------------------------------------------
              if (numLines == 0):
                 functions.samPrintFAILED()
                 print "Problem with hold command, no results returned"
                 functions.samNewLine()
                 sys.exit(2)

    #----------------------------------------------------------------------------------
    #Some results were returned, finally get the results of the blah_job_hold command
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
              returnCode = res[2]
              if (returnCode != '0'):
                functions.samPrintFAILED()
                print "blah_job_hold had a problem: " + returnCode
                functions.samNewLine()
                sys.exit(2)

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
                  jobstatus = res[5]

              functions.samPrintOK()
              print "The Job was successfully put on hold via the blah_job_hold command, the jobid is: " + jobid
              print "Job Status is: " + jobstatus
              functions.samNewLine()

#--------------------------------------------------------------------------------------------------------------
#Launch job resume command and get first result from output
#-----------------------------------------------------------------------------------------------------------

    #-------------------------------------------------------------------------------
    #Launch the job_hold command and get first result from output
    #-------------------------------------------------------------------------------
              os.write(fin.fileno(), 'blah_job_resume 3 ' + jobid + '\n')
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
    #There was a problem with the resume command - no results returned
    #------------------------------------------------------------------
              if (numLines == 0):
                 functions.samPrintFAILED()
                 print "Problem with job resume command, no results returned"
                 functions.samNewLine()
                 sys.exit(2)
                                                                                                                        
    #----------------------------------------------------------------------------------
    #Some results were returned, finally get the results of the blah_job_hold command
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
              returnCode = res[2]
              if (returnCode != '0'):
                functions.samPrintFAILED()
                print "blah_job_resume had a problem: " + returnCode
                functions.samNewLine()
                sys.exit(2)
              functions.samPrintOK()
              print "The Job was successfully resumed via the blah_job_resume command, the jobid is: " + jobid
              functions.samNewLine()
              os.write(fin.fileno(), 'quit\n')
              return 1

            else:
             print "Job could not be put on hold, its status is neither idle or running"
             functions.samNewLine()
             os.write(fin.fileno(), 'quit\n')
             return 0 


if __name__ == "__main__":
     #----------------
     # Start testing
     #---------------
    
    functions.samPrintINFO()
    print "Start testing the BLAH_JOB_HOLD and the BLAH_JOB_RESUME commands ..."
    functions.samNewLine()

    input = open("classadd_job", 'r')
    for line in input:
         jobad = line
         functions.samPrintINFO()
         print "Start testing the BLAH_JOB_HOLD and the BLAH_JOB_RESUME commands with classad: "
         functions.samNewLine() 
         print jobad
                                                                                                      
         if test_blah_job() :
            functions.samPrintPASSED()
            print "Blah_job_hold and Blah_job_resume completed!"
            functions.samNewLine()
