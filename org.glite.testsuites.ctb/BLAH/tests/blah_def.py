#!/usr/bin/python

#-----------------------------------------------------
#Some useful function definitions for the BLAH tester
#----------------------------------------------------

import os, time

def to_int(in_str):
#-----------------------------------
#Converts a string to an integer
#-----------------------------------
    out_num = 0
    if in_str[0] == "-":
        multiplier = -1
        in_str = in_str[1:]
    else:
        multiplier = 1
    for x in range(0,len(in_str)):
        out_num = out_num * 10 + ord(in_str[x]) - ord('0')
    return out_num * multiplier

def read_line(buff):
#------------------------------
#reads one line from the fd
#------------------------------
    line = ''
    while line[-1:] != '\n': line += os.read(buff.fileno(), 1)
    return line

def get_results(numLines,buffout,buffin):
#----------------------------------------------
#loop until some results are found or up to 40
#----------------------------------------------
    time.sleep(20)
    count = 0
    while ((numLines == 0) and (count <= 40)):
             os.write(buffin.fileno(), 'results\n')
             res = read_line(buffout)
             numLines = to_int(res.split()[-1])
             count += 1
    return res
 
