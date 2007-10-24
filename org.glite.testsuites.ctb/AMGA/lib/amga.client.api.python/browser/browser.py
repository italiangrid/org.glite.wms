#!/usr/bin/env python
#This file contains the main code of the browser and the lines to execute it.
#It imports base_graphics module where all the components of the gui are istantiated.



import sys

#sys.path.append("../")

import os.path
from os import popen2
import os
import threading
import time
import threading

import mdclient
from settings import *
import images

from qt import *
from qttable import QTable

from base_graphics import *


class startdashboard(threading.Thread):
	def __init__(self, program):
		threading.Thread.__init__(self)
		self.program=program
		self.linelist=[]
		self.pr_stdin, self.pr_stdout = os.popen2(self.program,"r")
		#self.pr_stdin, self.pr_stdout = popen2("ls","r")
		self.please_end=0
	def run(self):
		while 1:
			#print "please_end:",please_end
			if verbose: print "self.please_end:",self.please_end
			if self.please_end==1:
				#self.pr_stdin.close()
				break
			line = self.pr_stdout.readline()
			if self.linelist.count(line)==0:
				self.linelist.append(line)
				#print self.linelist

	def stop(self):
		if verbose: print "PLEASE STOP == 1"
		self.pr_stdin.write("exit")
		#time.sleep(1)
		self.pr_stdin.close()
		self.please_end=1
		#ext_end_dashboard()

#############################################################
#############################################################
		
class filenames(threading.Thread):
	def __init__(self, db):
		threading.Thread.__init__(self)
		self.directory="/"
		self.db=db
		self.please_end=0
		self.entry_list=[]
		if verbose: print "FILES THREAD ISTANTIATED"
		#self.run(db,directory)

	def run(self):
		# No attributes are needed now
		self.db.nattrs=1
		#execute command
		if verbose: print "dir "+self.directory
		self.db.execute("dir "+self.directory )

		while not self.db.eot() :
			#print "please_end: ",self.please_end
			if self.please_end==1:
			 #	print "names thread terminated"
				break
			workavailable.acquire()
			entry =  self.db.getEntry()
			workavailable.release()
			#print "names thread: ",entry
			self.entry_list.append(entry)

	def set_directory(self,directory):
		self.directory=directory

	def stop(self):
		#workavailable.acquire()
		self.please_end=1
		#workavailable.release()
		if verbose: print "please end names: ",self.please_end

##############################################################
##############################################################

class fileattributespanel:
	def __init__(self,mainwidget):
		self.layoutwidget = QDialog()
		layout = QVBoxLayout(self.layoutwidget)
		
		title_label = QLabel(self.layoutwidget)
		layout.addWidget(title_label)
		
		self.table =  QTable(self.layoutwidget)
		self.table.setReadOnly(1)
		self.table.setNumCols(2)
		self.table.horizontalHeader().setLabel(0,"Attribute Name")
		self.table.horizontalHeader().setLabel(1,"Attribute Value")
		layout.addWidget(self.table)
		
		ok_button= QPushButton(self.layoutwidget)
		ok_button.setText("OK")
		self.layoutwidget.connect(ok_button,SIGNAL("clicked()"),self.hide)
		layout.addWidget(ok_button)
		
		
	def setparams(self,filename,fileattrs,fileattrsvalues):
		self.layoutwidget.setCaption(filename)
		
		len_fileattrs=len(fileattrs)
		self.table.setNumRows(len_fileattrs)
		for i in range(len_fileattrs):
			self.table.setText(i,0,fileattrs[i]) ; self.table.setText(i,1,fileattrsvalues[i])
		
	def show(self):
		self.layoutwidget.setHidden(0)
	#	self.layoutwidget._raise()
	
	def hide(self):
		self.layoutwidget.setHidden(1)
	
##############################################################
##############################################################

class connectiondialog:
	def __init__(self,parent):
		
		self.parent= parent
	
		self.layoutwidget = QDialog(parent)
		self.layoutwidget.setGeometry(350,300,300,100)
		self.layoutwidget.setCaption("Connect to...")
		Vlayout = QVBoxLayout(self.layoutwidget)
		
		spacer = QSpacerItem(10,10)
		
		widget_address = QWidget(self.layoutwidget,"widget_address")
		Olayout_address = QHBoxLayout(widget_address)
		label_address=QLabel(widget_address)
		Olayout_address.addWidget(label_address)
		label_address.setText( "Server Address: ")
		self.linedit_address= QLineEdit(widget_address)
		self.linedit_address.setText("lxarda01.cern.ch")
		Olayout_address.addWidget(self.linedit_address)
		Vlayout.addWidget(widget_address)
		Vlayout.addItem(spacer)
		
		widget_port = QWidget(self.layoutwidget,"widget_port")
		Olayout_port = QHBoxLayout(widget_port)
		label_port=QLabel(widget_port)
		Olayout_port.addWidget(label_port)
		label_port.setText( "Port Number: ")
		self.linedit_port= QLineEdit(widget_port)
		self.linedit_port.setText("8822")
		Olayout_port.addWidget(self.linedit_port)
		Vlayout.addWidget(widget_port)
		Vlayout.addItem(spacer)
		
		widget_username = QWidget(self.layoutwidget,"widget_username")
		Olayout_username = QHBoxLayout(widget_username)
		label_username=QLabel(widget_username)
		Olayout_username.addWidget(label_username)
		label_username.setText( "Username: ")
		self.linedit_username = QLineEdit(widget_username)
		self.linedit_username.setText("root")
		Olayout_username.addWidget(self.linedit_username)
		Vlayout.addWidget(widget_username)
		Vlayout.addItem(spacer)
		
		connect_button = QPushButton(self.layoutwidget)
		connect_button.setText("Connect!")
		
		Vlayout.addWidget(connect_button)
		self.layoutwidget.connect(connect_button,SIGNAL("clicked()"),self.srv_connect)
		
		self.layoutwidget.show()

#-----------------------------------------------------------------------------------------------------------
				
	def srv_connect(self):
		self.server_name = str(self.linedit_address.text())
		self.port_number=int(str(self.linedit_port.text()))
		self.username=str(self.linedit_username.text())
		self.layoutwidget.setHidden(1)
		
		self.parent.build_browser()
		
#--------------------------------------------------------------------------------------------------------

	def show(self):
		
		self.layoutwidget.show()

##############################################################
##############################################################

class browser(base_graphics):
	def __init__(self):
		base_graphics.__init__(self)

		# Dictionary to store the full paths of files and directories
		self.full_names_dict={}
		
		self.connection_dialog=connectiondialog(self)
		#self.build_browser()

		# File Menu
		self.connect(self.fileExitAction,SIGNAL("activated()"),self.end_threads)
		self.connect(self.ConnectAction,SIGNAL("activated()"),self.serv_connect)
		self.connect(self.DisconnectAction,SIGNAL("activated()"),self.disconnect)
		self.connect(self.TablesaveAction,SIGNAL("activated()"),self.showsavefilewidget)
		# Opts Menu
		self.connect(self.EraseHistoryAction,SIGNAL("activated()"),self.erasehistory)
		self.connect(self.ToggleShellAction,SIGNAL("activated()"),self.toggle_shell)
		self.connect(self.ToggleTableAction,SIGNAL("activated()"),self.toggle_table)
		# Upper right cross
		self.connect(a,SIGNAL("lastWindowClosed()"),self.end_threads)
		
		# A flag to remeber if it's the first time the Browser Graphics is built
		self.first_time_browser=1
		

#----------------------------------------------------------------------------------------------------------

	def build_browser(self):
		if self.first_time_browser==0:
			del self.timer
			del self.timer2
			self.full_names_dict={}
		
		if verbose: print "Before connection"
			
		self.db = mdclient.MDClient\
			(self.connection_dialog.server_name,\
			self.connection_dialog.port_number,\
			self.connection_dialog.username)
		
		if verbose: print "After connection"
		
		# Panel to show file attributes
		self.file_attributes_panel=fileattributespanel(self)
		# List of browsed dirs for listview
		self.list_browsed_dirs=[]
		# Update the Listview:
		initial_item=QListViewItem(self.middle_widget.listview,"/","collection")
		initial_item.setPixmap(0,self.small_dir_image_close)
		self.full_names_dict.setdefault(initial_item,["/","/","collection"])
		self.fill_listview("/",initial_item)
		initial_item.setOpen(1)
		
		# Set previous selected item to 0 for initialisation.
		self.previous_selected_item=0
		# Replica of the selections on a list
		self.selection_list=[]
		# Fill selections combobox from file: .selection_history
		self.fill_sel_from_file()
		# Lista attributi
		self.attr_list=[]
		# Start AMGA:
		self.number_of_lines=0
		self.connect(self.bottom_widget.textedit,SIGNAL("returnPressed()"),self.textedit_return_pressed)
		self.connect(self.bottom_widget.textedit,SIGNAL("textChanged()"),self.textedit_cur_pos_changed)
		self.startAMGA()
		# Threading class to retrieve filenames
		self.file_names=filenames(self.db)
		
		# A timer to periodically call periodicCall :-)
		self.timer = QTimer()
		QObject.connect(self.timer,SIGNAL("timeout()"),self.periodicCall)
		 # Start the timer -- this replaces the initial call to periodicCall
		self.timer.start(11)
		
		# A flag to know if the user is waiting for a sellection of files:
		self.go_pressed=0
		
		# A timer to periodically call periodicCall2 :-)
		self.timer2 = QTimer()
		QObject.connect(self.timer2,SIGNAL("timeout()"),self.periodicCall2)
		 # Start the timer -- this replaces the initial call to periodicCall
		self.timer2.start(300)
		
		# A save file dialog to save the contents of the table-
		self.setupsavedialog()
		
		# Connections:
		# listview
		self.connect(self.middle_widget.listview,SIGNAL("clicked(QListViewItem*)"),self.if_diropen)
		#self.connect(self.middle_widget.listview,SIGNAL("clicked(QListViewItem*)"),self.showattr)
		# Send Query button
		self.connect(self.middle_widget.go_button,SIGNAL("clicked()"),self.send_query)
		# Stop Button
		self.connect(self.middle_widget.stop_button,SIGNAL("clicked()"),self.cancel)
		# Table
		self.connect(self.middle_widget.selection_table,SIGNAL("selectionChanged()"),self.show_item_attrs)
		# Save Dialog
		self.connect(self.savedialog,SIGNAL("fileSelected(const QString &)"),self.savetableonfile)
		
		# PATHBAR
		self.connect(self.path_widget.save_button,SIGNAL("clicked()"),self.showsavefilewidget)
		self.connect(self.path_widget.connect_button,SIGNAL("clicked()"),self.serv_connect)
		self.connect(self.path_widget.disconnect_button,SIGNAL("clicked()"),self.disconnect)
		
		# A flag to remeber if it's the first time the Browser Graphics is built
		self.first_time_browser=0
#----------------------------------------------------------------------------------------------------------

	def show_item_attrs(self):
		
		current_row=self.middle_widget.selection_table.currentRow()
		file_name= str(self.middle_widget.selection_table.text(current_row,0))
		n_cols=self.middle_widget.selection_table.numCols()
		if verbose: print n_cols
		file_attrs=[]
		if n_cols>1:
			for i in range(1,n_cols):
				file_attrs.append(str(self.middle_widget.selection_table.text(current_row,i)))
		if n_cols==1:
			self.db.getattr(str(self.path_widget.pathbar.text())+"/"+file_name,self.attr_list)
			file_attrs =  self.db.getEntry()[1]
		
		print file_name
		print file_attrs
		self.file_attributes_panel.setparams(file_name,self.attr_list,file_attrs)
		self.file_attributes_panel.show()
		#self.attr_list
		
#----------------------------------------------------------------------------------------------------------

	def toggle_shell(self):
		if self.bottom_widget.textedit.isHidden():
			self.ToggleShellAction.setIconSet(QIconSet(self.tick))
			self.bottom_widget.textedit.setShown(1)
			self.bottom_widget.texteditlabel.setShown(1)
		else:
			self.ToggleShellAction.setIconSet(QIconSet(self.circle))
			self.bottom_widget.textedit.setShown(0)
			self.bottom_widget.texteditlabel.setShown(0)
	
#----------------------------------------------------------------------------------------------------------

	def toggle_table(self):
		if self.middle_widget.selection_table.isHidden():
			self.ToggleTableAction.setIconSet(QIconSet(self.tick))
			self.middle_widget.selection_table.setShown(1)
		else:
			self.ToggleTableAction.setIconSet(QIconSet(self.circle))
			self.middle_widget.selection_table.setShown(0)
			
#----------------------------------------------------------------------------------------------------------

	def periodicCall2(self):
		# if the user is waiting for a selection do nothing
		if self.go_pressed==1: return 1

		self.middle_widget.selection_table.setNumCols(1)
		self.middle_widget.selection_table.horizontalHeader().\
				setLabel(0,self.__tr("File Name"))
		
		counter=1
		break_flag=0
		workavailable.acquire()
		for entry in self.file_names.entry_list:
			#print "absrowindex: ",self.middle_widget.abs_row_index
			if verbose: print entry
			
			counter+=1
			if counter>n_table_items_per_periodic_call:
				break_flag=1
				break
			
			name=os.path.basename(entry[0])
			self.middle_widget.selection_table.setNumRows(self.middle_widget.abs_row_index+1)
			self.middle_widget.selection_table.setText(self.middle_widget.abs_row_index,0,name)
			self.middle_widget.abs_row_index+=1
		if break_flag==1:
			if verbose: print "Limit of entries added to  the table per periodic call reached."
			self.file_names.entry_list=self.file_names.entry_list[n_table_items_per_periodic_call:]
		else:
			self.file_names.entry_list=[]
		workavailable.release()
		
#----------------------------------------------------------------------------------------------------------

	def setupsavedialog(self):
		"""
		Sets up the dialog to save the table in a file
		"""
		self.savedialog = QFileDialog()
		# Allows to save on non existing files 
		self.savedialog.setMode(QFileDialog.AnyFile)
		self.savedialog.setDir(os.path.expanduser("~"))
		self.savedialog.setFilters("HTML files (*.html)")
		self.savedialog.setFilter("ASCII comma separated files (*.txt)")
		self.savedialog.setCaption("Save Table as..")

#----------------------------------------------------------------------------------------------------------

	def erasehistory(self):
		if os.path.isfile(history_file_name):
			os.remove(history_file_name)
			if verbose: print history_file_name + " successfully deleted!"
		else: print history_file_name + " not present!"
		self.fill_sel_from_file()
		
#-----------------------------------------------------------------------------------------------------------
		
	def end_threads(self):
		"""
		Ends the active threads
		"""
		self.dashboard.stop()
		self.file_names.stop()
		
#-----------------------------------------------------------------------------------------------------------
		
	def serv_connect(self):
		self.connection_dialog.show()
		
#-----------------------------------------------------------------------------------------------------------

	def disconnect(self):
	# Avoid the further refreshing of table contents bu periodicCall2
		self.go_pressed=1
	# Hide table Rows and Columns
		for counter in range(self.middle_widget.selection_table.numCols()):
			self.middle_widget.selection_table.removeColumn(counter)
		for counter in range(self.middle_widget.selection_table.numRows()):
			self.middle_widget.selection_table.removeRow(counter)
		self.middle_widget.selection_table.setNumRows(0)
	# End Threads
		self.end_threads()
	# Clear ListView
		self.middle_widget.listview.clear()
	# Delete connection object
		del self.db

#-----------------------------------------------------------------------------------------------------------

	def send_query(self):
	
		del self.db
		self.db = mdclient.MDClient\
			(self.connection_dialog.server_name,\
			self.connection_dialog.port_number,\
			self.connection_dialog.username)
			
		self.go_pressed=1
		self.write_sel_on_file()
		selections = str(self.middle_widget.selcombobox.currentText())
		selected_dir = str(self.middle_widget.listview.currentItem().text(0))

		fullfilename=self.full_names_dict[self.middle_widget.listview.currentItem()][0]
		if verbose: print "Fullfilename"
		if verbose: print fullfilename
		
		base,head = os.path.split(fullfilename)
		if base == "/":
			selected_dir=base+head
		else: selected_dir=base
		
		if verbose: print selected_dir

		self.middle_widget.selection_table.setNumRows(0)
		self.middle_widget.selection_table.setNumCols(1)
		file_name_dict={}
		counter=0
		
		# Column number Settings
		len_attr_list=len(self.attr_list)
		numcols =len_attr_list + 1 # All the attributes plus the name of the file
		self.middle_widget.selection_table.setNumCols(numcols)
		if verbose: print self.attr_list
		self.middle_widget.selection_table.horizontalHeader().\
				setLabel(0,self.__tr("File Name"))
		
		counter=1
		for attr in self.attr_list:
			if verbose: print attr
			self.middle_widget.selection_table.horizontalHeader().\
				setLabel(counter,self.__tr(attr))
			counter+=1
		
		# Build the command from atttributes
		if selections==default_selcomboboxitem or selections=='':
			command = "find " + selected_dir + "/*" 
		else:
			command = "find " + selected_dir + "/* \'" 
			list_of_selections=selections.split(" ")
			len_list_of_selections=len(list_of_selections)
			for selection in list_of_selections:
				command +=selected_dir+":"+selection+ " "
			command += "\'"
		if verbose: print command
		
		# No attributes are needed now
		self.db.nattrs=0
		#execute command
		self.db.execute(command)
		
		while not self.db.eot():
			entry =  self.db.getEntry()
		#for entry in results_content.entry_list:
			if verbose: print entry
			item_name=entry[0]
			#print item_name
			full_name= selected_dir+"/"+item_name
			file_name_dict.setdefault(item_name,full_name)
			self.middle_widget.selection_table.setNumRows(self.middle_widget.selection_table.numRows()+1)
			self.middle_widget.selection_table.setText(0,counter,item_name)
			counter+=1
		
		# Put the entries in the table:
		row_counter=0
		for item_name in file_name_dict.iterkeys():
			full_item_name=file_name_dict[item_name]
			#print full_item_name
			self.db.getattr(full_item_name,self.attr_list)
			attributes_list =  self.db.getEntry()[1]
			#print attributes_list
			self.middle_widget.selection_table.setText(row_counter,0,item_name)
			col_counter=1
			for attr in attributes_list:
				self.middle_widget.selection_table.setText(row_counter,col_counter,str(attr))
				
				col_counter+=1
			row_counter+=1
	
#-----------------------------------------------------------------------------------------------------------

	def showsavefilewidget(self):
		"""
		A function to show a widget to save the contents of the table.
		"""
		self.savedialog.show()
	
#-----------------------------------------------------------------------------------------------------------

	def savetableonfile(self):
		"""
		Creates an HTML file or plain text file containing the table 
		'"""
		# Open the file to write the table on:
		selected_file = str(self.savedialog.selectedFile())
		# If it does not terminate with html, add the suffix
		is_html=0
		suffix=selected_file[-5:]
		if suffix==".html":
			is_html=1
		ofile = file(selected_file,"w")
		
		# Get the dimensions of the table
		table_width=self.middle_widget.selection_table.numCols()
		table_height=self.middle_widget.selection_table.numRows()

		if is_html:
			# HTML headers:
			ofile.write("<html>")
			ofile.write("<font size=\"2\">")
			ofile.write("<table border=\"1\">\n")
		
			# Write the headers of each column
			# File name
			ofile.write( "<tr>\n<th>File Name</th>")
			# Other Attributes
			for attr in self.attr_list:
				ofile.write("<th>"+attr+"</th>")
			ofile.write("\n</tr>")
		
			# Iterate over table elements and print the contents on the file
			for y in range(table_height):
				ofile.write( "<tr>")
				for x in range(table_width):
					#print "x,y",x," ",y,"\n"
					ofile.write("<td>"+str(self.middle_widget.selection_table.item(y,x).text())+"</td>")
				ofile.write("\n</tr>")
		
			# HTML footer
			ofile.write("</table>")
			ofile.write("</font size=-2>")
			ofile.write("<span style=\"font-style: italic;\">")
			ofile.write("Automatically generated with <B>AMGA General Browser</B> on ")
			ofile.write(str(time.ctime()))
			ofile.write("</span>\n")
			ofile.write("</html>")
			
			ofile.close()
		
		else:
			separator=" , "
			counter=0
			for attr in self.attr_list:
				ofile.write(attr)
				counter+=1
				if counter!=len(self.attr_list):
					ofile.write(separator)
			ofile.write("\n")
			# Iterate over table elements and print the contents on the file
			counter=0
			for y in range(table_height):
				for x in range(table_width):
					ofile.write(str(self.middle_widget.selection_table.item(y,x).text())+separator)
					counter+=1
					if counter!=table_width:
						ofile.write(separator)
				ofile.write("\n")
		
#-----------------------------------------------------------------------------------------------------------

	def textedit_cur_pos_changed(self):
		"""
		If the modified text is not at the last line,goto to the last line.
		"""
		doctext= self.bottom_widget.textedit.document()
		cur_line, cur_x = self.bottom_widget.textedit.getCursorPosition()
		nlines = self.bottom_widget.textedit.lines()
		if self.number_of_lines<nlines:
			self.number_of_lines=nlines
		#print nlines
		current_text_list=str(self.bottom_widget.textedit.text()).split("\n")
		if self.number_of_lines-1 != cur_line or (current_text_list[cur_line] == mdclient_prompt[:-1]):
			self.bottom_widget.textedit.undo()
			self.bottom_widget.textedit.setCursorPosition(nlines-1,len(mdclient_prompt)+1)

		
#-----------------------------------------------------------------------------------------------------------
	
	def textedit_return_pressed(self):
		current_text_list=str(self.bottom_widget.textedit.text()).split("\n")
		#print current_text_list
		len_current_text_list=len(current_text_list)
		#last_line = current_text_list[len_current_text_list-3]
		last_line = current_text_list[ -2 ]
		#print "last_line "+ last_line

		workavailable.acquire()
		command = last_line[len(mdclient_prompt):]
		if verbose: print "command=",command, "command[:2]=",command[:2] 
		self.dashboard.pr_stdin.write(command+"\n")
		self.dashboard.pr_stdin.flush()
		workavailable.release()
		if command[:2] == "cd" :
			self.dashboard.linelist.append(mdclient_prompt)
			#self.bottom_widget.textedit.append(mdclient_prompt)
	
#-----------------------------------------------------------------------------------------------------------

	def periodicCall(self):
		workavailable.acquire()
		list_of_lines=self.dashboard.linelist[:5]
		#self.dashboard.linelist=[]
		self.dashboard.linelist=self.dashboard.linelist[5:]
		workavailable.release()

		len_list_of_lines=len(list_of_lines)
		if len_list_of_lines!=0:
			for i in range(len_list_of_lines):
				self.bottom_widget.textedit.append(list_of_lines[-i])

			# write the prompt
			if self.dashboard.linelist ==[]:
				if list_of_lines[-1]!=mdclient_prompt:
					self.bottom_widget.textedit.append(mdclient_prompt)

			# set cursor pos at the end of the window
			nlines=self.bottom_widget.textedit.lines()
			self.bottom_widget.textedit.setCursorPosition(nlines-1,len(mdclient_prompt)+1)

#--------------------------------------------------------------------------------------------------------

	def startAMGA(self):

		self.dashboard = startdashboard(mdclient_command+" "+self.connection_dialog.server_name)
		self.dashboard.start()

#-----------------------------------------------------------------------------------------------------------

	def fill_sel_from_file(self):

		self.middle_widget.selcombobox.clear()
		self.middle_widget.selcombobox.insertItem(default_selcomboboxitem)
		if not os.path.isfile(history_file_name): return 1

		history_file = open(history_file_name,"r")
		commands =  history_file.readlines()
		if verbose: print commands
		n=len(commands)
		i=n-1
		while i != -1 and i!=n-number_of_recorded_commands :
	 	#print "n="+str(n)+" i="+str(i)
	 	#print commands[i]
			command = commands[i][:-1]
			self.middle_widget.selcombobox.insertItem(command)
			self.selection_list.append(command)
			i-=1

#-----------------------------------------------------------------------------------------------------------

	def write_sel_on_file(self):

		selection = str(self.middle_widget.selcombobox.currentText())
		if self.selection_list.count(selection) == 0:
			if selection == default_selcomboboxitem:
				return 1
			if os.path.isfile(history_file_name): history_file = open(history_file_name,"a") 
			else: history_file = open(history_file_name,"w")
			history_file.write(selection+"\n")
			self.middle_widget.selcombobox.insertItem(selection)
			if verbose: print selection
			history_file.close()

#-----------------------------------------------------------------------------------------------------------

	def fill_listview(self,dir_to_list,current_item):

		if self.list_browsed_dirs.count(dir_to_list)!=0:
			return 1
		self.list_browsed_dirs.append(dir_to_list)
		self.db.listEntries(dir_to_list)

		print "inside filllistview"

		# Put an upper limit to the items read from the server
		counter = max_read_entries_number

		while not self.db.eot():
			if counter == 0:
				if verbose: print "Maximum number of items in listview reached!"
				del self.db
				self.db = mdclient.MDClient\
				(self.connection_dialog.server_name,\
				self.connection_dialog.port_number,\
				self.connection_dialog.username)
				break	
			entry =  self.db.getEntry()
			item_name=entry[0]
			#print item_name
			item_type=entry[1]
			# Crop the '/' and the full path
			tidy_item_name=os.path.split(item_name)[1]
			if item_type[0] == "collection":
				item=QListViewItem(current_item,tidy_item_name,item_type[0])
				item.setPixmap(0,self.small_dir_image_close)
				self.full_names_dict.setdefault(item,[item_name,tidy_item_name,item_type[0]])
			#else: item.setPixmap(0,self.small_file_image)
			#self.full_names_dict.setdefault(item,[item_name,tidy_item_name,item_type[0]])
			elif  item_type[0] != 'entry':
				if verbose: print "different from collection and entry"
				item=QListViewItem(current_item,tidy_item_name,item_type[0])
				item.setPixmap(0,self.file_small_image)
				self.full_names_dict.setdefault(item,[item_name,tidy_item_name,item_type[0]])
			counter -=1
			

#-----------------------------------------------------------------------------------------------------------

	def if_diropen(self):
		
		self.showattr()
		time.sleep(.1)

		current_item=self.middle_widget.listview.currentItem()
		current_name=self.full_names_dict[current_item][0]
		current_tidy_item_name=self.full_names_dict[current_item][1]
		current_type=self.full_names_dict[current_item][2]

		# Refresh the current dir name:
		self.path_widget.pathbar.setText(current_name)
							
		if current_item.isOpen():
			current_item.setOpen(0)
			return
		
		if verbose: print current_name
		
		if current_type == 'collection':
			self.fill_listview(current_name,current_item)
			current_item.setOpen(1)
			
			# Change icon of selected item and restore the old selected one.
			if self.previous_selected_item !=0:
				self.previous_selected_item.setPixmap(0,self.small_dir_image_close)
			# This line to keep trace of the previous selected item
			self.previous_selected_item=current_item
			
			current_item.setPixmap(0,self.small_dir_image_open)
		
			# Sincronize the current dit on mdclient shell
			self.dashboard.pr_stdin.write("cd "+current_name+"\n")
			self.dashboard.pr_stdin.write("pwd\n")
			self.dashboard.pr_stdin.flush()
			
			# THREAD MANAGEMENT
			self.middle_widget.abs_row_index=0
			#workavailable.acquire()
			self.file_names.stop()
			#workavailable.release()
			
			
			#self.cancel()
			del self.file_names
			del self.db
			self.db = mdclient.MDClient\
			(self.connection_dialog.server_name,\
			self.connection_dialog.port_number,\
			self.connection_dialog.username)
			
			self.file_names = filenames(self.db)
			self.file_names.set_directory(current_name)
			
			print "start name thread"
			self.file_names.start()
			
			# END THREAD MANAGEMENT
			
#-----------------------------------------------------------------------------------------------------------

	def showattr(self):

		#self.cancel()
		del self.db
		self.db = mdclient.MDClient\
			(self.connection_dialog.server_name,\
			self.connection_dialog.port_number,\
			self.connection_dialog.username)
			
		self.go_pressed=0
		self.middle_widget.abs_row_index=0
		
		current_item = self.middle_widget.listview.currentItem()
		current_item_name = self.full_names_dict[current_item][0]
		current_item_tidy_name=self.full_names_dict[current_item][1]
		current_item_type = self.full_names_dict[current_item][2]
		if verbose: print "current_item_name:",current_item_name
		attributes= str(self.db.listAttr(current_item_name))
		if attributes=='([], [])':
			attributes='None'
			return 1
		#current_item.setText(2,attributes)
		#print attributes#.split(",\',")
		last_par = attributes.find ("]")
		self.attr_list = attributes[3:last_par-1].split('\', \'')
		if verbose: print self.attr_list
		len_attrlist=len(self.attr_list)

		if current_item_type == "entry":
			self.db.getattr(current_item_name,self.attr_list)
			attributes_values =  self.db.getEntry()[1]

			counter=0
			for attribute_number in range(len_attrlist):
				attributes_values[attribute_number]= self.attr_list[attribute_number]+" "+attributes_values[attribute_number]
				counter+=1
			# Attributes Labels
			if verbose: print attributes_values
		
		#RadioButtons
		
		#del self.middle_widget.radio_buttons
		#self.middle_widget.radio_buttons = selectionradiobuttons(self.middle_widget.middleverticalayoutwidget,self.middle_widget.middleverticalayout)
		self.middle_widget.radio_buttons.set_buttons(self.attr_list)
		
		# Establish Connections
		for button in self.middle_widget.radio_buttons.list_radiobuttons:
			self.connect(button,SIGNAL("stateChanged(int)"),self.arrange_table_columns)

#-----------------------------------------------------------------------------------------------------------

	def cancel(self):
		
		# If the thread to retreive names is alive, terminate it
		if self.file_names.isAlive():
			workavailable.acquire()
			self.file_names.stop()
			workavailable.release() 
			
		# Stop refreshing the table
		self.go_pressed=1
		
		#the CAN byte: 0x18
		can_string="\030"
		if verbose: print "Sending CAN Byte"
		
		# Send the CAN byte
		self.db.s.send(can_string)
		
		# Now Flush the buffer: if it takes more than 5 seconds, disconnect and reconnect.
		if verbose: print "Flushing buffer"
		start_time=time.time()
		while not self.db.eot():
			time_interval = int(time.time() - start_time)
			if verbose: print "Flush buffer Time Interval: ",time_interval 
			if time_interval == 3:
				if verbose: print "Time exceeded 3 seconds, reconnecting.."
				del self.db
				self.db = mdclient.MDClient\
				(self.connection_dialog.server_name,\
				self.connection_dialog.port_number,\
				self.connection_dialog.username)
				break
			self.db.getEntry()

#------------------------------------------------------------------------------------------------------------

	def arrange_table_columns(self):
		counter =1
		for button in self.middle_widget.radio_buttons.list_radiobuttons:
			if button.isChecked():
				self.middle_widget.selection_table.showColumn(counter)
			else: self.middle_widget.selection_table.hideColumn(counter)
			counter+=1

#-----------------------------------------------------------------------------------------------------------

	def __tr(self,s,c = None):
		return qApp.translate("Form3",s,c)


if __name__ == "__main__":
	a = QApplication(sys.argv)
	w = browser()
	QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
	QObject.connect(w.fileExitAction,SIGNAL("activated()"),a,SLOT("quit()"))
	a.setMainWidget(w)
	w.show()
	a.exec_loop()
