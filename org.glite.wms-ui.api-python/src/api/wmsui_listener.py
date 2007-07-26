#! /usr/bin/env python
"""
***************************************************************************
    filename  : wmsui_listener.py
    author    : Alessandro Maraschini
    email     : egee@datamat.it
    copyright : (C) 2003 by DATAMAT
***************************************************************************
//
// $Id$
//
"""
from wmsui_api import Shadow
from wmsui_api import JobId
from glite_wmsui_AdWrapper import AdWrapper
import thread
import sys
try:
	from Tkinter import *
	tkint =1
except:
	tkint=0

"""
#################################
ListenerFrame Class
#################################
"""
class ListenerFrame:
  """
  C-tor
  """
  def __init__(self, master, shadow):
	if not tkint:
		print "***Fatal Error*** ListenerFrame __init__: No Python graphical libraries found.\
		\nUnable to create graphical component: system exiting"		
		sys.exit(1)
	self.shadow = shadow
	frame = Frame(master)
	frame.pack()
	Label (frame , text='JobId:').pack()
	jobidText = Text (frame, height=1,width=60)
	jobidText.insert(END, shadow.jobid.jobid )
	jobidText.config(state=DISABLED)
	jobidText.pack()

	# Adding std output
	Label (frame , text='Standard Output:').pack( )
	outFrame = Frame(frame)
	outScrollbar = Scrollbar( outFrame)
	outScrollbar.pack(side=RIGHT, fill=Y)
	self.outputText = Text (outFrame , height=10,width=60 , background ="white",padx = 5 , pady = 3, yscrollcommand=outScrollbar.set )
	outScrollbar.config(command=self.outputText.yview)
	self.outputText.config(state=DISABLED)
	self.outputText.pack(side=LEFT)
	outFrame.pack()

	# Adding std output
	Label (frame , text='Standard Error:').pack()
	errFrame = Frame(frame)
	errScrollbar = Scrollbar( errFrame)
	errScrollbar.pack(side=RIGHT, fill=Y)
	self.errText = Text (errFrame , height=6,width=60, background ="white", padx = 5 , pady = 3, yscrollcommand=errScrollbar.set  )
	errScrollbar.config(command=self.errText.yview)
	self.errText.config(state=DISABLED)
	self.errText.pack(side=LEFT)
	errFrame.pack()

	# Adding send TextField
	Label (frame , text='Sending standard input:').pack()
	self.inputText = Text (frame , height=2,width=60, background ="white" , padx = 5 , pady = 6)
	self.inputText.pack()
	# Add Send Button
	self.sendButton = Button(frame, text="Send", command=self.send).pack(side=RIGHT)
	Button(frame, text="Quit", command=self.detach).pack(side=LEFT)
	self.frame=frame
	self.master= master
	self.refresh=1
  """
  empty method: wait the Output pipe for stream
  """
  def emptyOut( self,  text):
     while (self.refresh):
	try:
		msg = self.shadow.emptyOut()
	except IOError:
		msg="\n FATAL ERROR: Unable to find listening pipe:\n" + self.shadow.getPipeOut()
		msg+="\n Please exit"
		text=self.errText
		self.refresh=0
	if msg:
		text.config(state=NORMAL)
		text.insert(END,msg)
		text.config(state=DISABLED)
  """
  empty method: wait the Error pipe for stream
  """
  def emptyErr( self,  text):
  	self.emptyOut(text)

  """
  run
  """
  def run(self):
	self.master.title( "Interactive Job Console" )
	#self.shadow.openPipes()
	thread.start_new_thread( self.emptyOut , (self.outputText, ) )
	#thread.start_new_thread( self.empty , (self.shadow.getPipeErr()  , self.errText ) ) #DERPRECATED
	self.master.mainloop()
	return 0

  """
  detach
  """
  def detach(self):
	self.inputText.delete(1.0, END)
	self.refresh = 0
	self.shadow.detach()
	self.frame.quit()
  """
  Send the message to the input pipe stream
  """
  def send(self):
     str = self.inputText.get (1.0, END)
     if self.shadow.writing:
	   self.errText.config(state=NORMAL)
	   self.errText.insert(END, "\n\n**** Error: Unable to send the message '"+str+"'.\nStill waiting for previous string  beeing read\n")
	   self.errText.config(state=DISABLED)
     else:
	if str:
	   self.inputText.delete(1.0, END)
	   self.outputText.config(state=NORMAL)
	   self.outputText.insert(END, "\n$ " + str)
	   self.outputText.config(state=DISABLED)
	   thread.start_new_thread( self.shadow.write ,  ( str,  )  )

"""
Non-gui listener component
"""
class Listener:

  ad = AdWrapper ()

  def __init__(self, shadow):
	self.shadow= shadow

  def emptyOut (self , void ):
	while 1:
		try:
			char = self.shadow.emptyOut()
			if char:
				self.ad.printChar (char)
		except IOError:
			print "***************************************"
			print " FATAL ERROR: Unable to find listening pipe:\n" + self.shadow.getPipeOut()
			print "\n Please exit session (^C)"
			print "***************************************"
			break


  def run(self):
    print "***************************************"
    print "Interactive Job console started for " + self.shadow.jobid.jobid
    print "Please press ^C to exit from the session"
    print "***************************************"
    try:
      thread.start_new_thread( self.emptyOut  , ( 0,) )
      self.sending()
    except KeyboardInterrupt:
        print "***************************************"
        print "Interactive Session ended by user"
        print "Removing Listener and input/output streams..."
        self.shadow.detach()
        print "Done"
        print "***************************************"

  """
  Send the message to the input pipe stream
  """
  def sending(self):
     while 1:
         ans=raw_input(  ).strip()
	 if ans:
           if self.shadow.writing:
              print "\n\n**** Error: Unable to send the message '"+ ans +"'.\nStill waiting for previous input beeing read\n"
           else:
              thread.start_new_thread(self.shadow.write,(ans,))
