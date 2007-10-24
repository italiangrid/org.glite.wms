#This file contains all the classes necessary to build up the GUI. The code 
#pertaining the AMGA interface is in browser.py file.


from settings import *
import images
from qt import *
from qttable import QTable
from settings import *


class base_graphics (QMainWindow):
	"""
	This class contains only the graphical information about the GUI: position of buttons,
	windows' sizes, menubars etc.
	"""
	def __init__(self,parent = None,name = None,fl = 0):
		QMainWindow.__init__(self,parent,name,fl)
		self.statusBar()
		
		if not name:
			self.setName("AGO: AMGA General brOwser")
		#self.setMaximumSize(800,600)
			
		# The images appearing in the gui
		self.istantiate_images()
		
		# Start Menu Bar --------------------------------------------------------------
		self.menubar = QMenuBar(self,"menubar")
		# File  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		self.build_file_menu()
		# Options  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		self.build_options_menu()
		# End Menu Bar -----------------------------------------------------------------
		
		# Start QSplitter
		self.splitter1 = QSplitter(Qt.Vertical, self, "split1")
		self.setCentralWidget(self.splitter1)
		# Class containing the current path and a label. It will be placed on the top of the window
		self.path_widget = pathwidget(self.splitter1,self.connect_image,self.disconnect_image,self.save_image)
		self.middle_widget = middlewidget(self.splitter1)
		self.bottom_widget = bottomwidget(self.splitter1)
		# End layout -------------------

		self.languageChange()

		self.resize(QSize(window_width,window_height).expandedTo(self.minimumSizeHint()))
		self.clearWState(Qt.WState_Polished)
		
	# CONNECTIONS
		
#----------------------------------------------------------------------------------------------

	def istantiate_listofattributes(self,vlayout,vlayoutwidget):
		listofattributes = QLabel (vlayoutwidget, "AttributesTextlabel")
		listofattributes.setGeometry(QRect(0,0,20,20))
		return listofattributes

#----------------------------------------------------------------------------------------------

	def istantiate_images(self):
		self.save_image = QPixmap(images.image2_data)
		self.tick = QPixmap(images.tick)
		self.circle = QPixmap(images.circle)
		
		self.connect_image = QPixmap (images.connect_image)
		self.disconnect_image = QPixmap (images.disconnect_image)
		
		self.file_small_image = QPixmap (images.file_small_image_data)
		self.small_dir_image_close = QPixmap (images.little_folder_closed)
		self.small_dir_image_open = QPixmap (images.little_folder_open)

		
#-----------------------------------------------------------------------------------------------
		
	def build_file_menu(self):
		self.ConnectAction = QAction(self,"Connect")
		self.DisconnectAction = QAction(self,"Disconnect")
		self.fileExitAction = QAction(self,"fileExitAction")
		
		self.TablesaveAction = QAction(self,"TableSaveAction")
		self.TablesaveAction.setIconSet(QIconSet(self.save_image))
		
		self.fileMenu = QPopupMenu(self)
		self.ConnectAction.addTo(self.fileMenu)
		self.DisconnectAction.addTo(self.fileMenu)
		self.TablesaveAction.addTo(self.fileMenu)
		self.fileMenu.insertSeparator()
		self.fileExitAction.addTo(self.fileMenu)
		
		self.menubar.insertItem(QString("File"),self.fileMenu,1)
		
#-----------------------------------------------------------------------------------------------
		
	def build_options_menu(self):
		
		self.EraseHistoryAction = QAction(self,"EraseHistoryAction")
		self.ToggleShellAction = QAction(self,"ToggleShellAction")
		self.ToggleShellAction.setIconSet(QIconSet(self.tick))
		self.ToggleTableAction = QAction(self,"ToggleTableAction")
		self.ToggleTableAction.setIconSet(QIconSet(self.tick))
		
		self.optsMenu = QPopupMenu(self)
		self.ToggleShellAction.addTo(self.optsMenu)
		self.ToggleTableAction.addTo(self.optsMenu)
		self.optsMenu.insertSeparator()
		self.EraseHistoryAction.addTo(self.optsMenu)
		
		self.menubar.insertItem(QString("Options"),self.optsMenu,1)
		
#-----------------------------------------------------------------------------------------------	
	def languageChange(self):
	
		self.setCaption(self.__tr("AGO: AMGA General brOwser"))
		# Start Menu Bar
		# File 
		self.ConnectAction.setText(self.__tr("Connect"))
		self.ConnectAction.setIconSet(QIconSet(self.connect_image))
		self.DisconnectAction.setText(self.__tr("Disconnect"))
		self.DisconnectAction.setIconSet(QIconSet(self.disconnect_image))
		self.fileExitAction.setText(self.__tr("Exit"))
		self.TablesaveAction.setText(self.__tr("Save Table As.."))
		# Opts
		self.EraseHistoryAction.setText(self.__tr("Erase History"))
		self.ToggleShellAction.setText(self.__tr("View Shell"))
		self.ToggleTableAction.setText(self.__tr("ViewTable"))
		
		self.fileExitAction.setAccel(QString.null)

#---------------------------------------------------------------------------------------------------------
	
	def __tr(self,s,c = None):
		return qApp.translate("Form3",s,c)
		
##############################################################
##############################################################

class bottomwidget:
	def __init__(self,splitter1):
	
		bottomlayoutwidget = QWidget(splitter1,"middlelayoutwidget")
		#bottomlayoutwidget.setMinimumHeight(100)
		bottomlayout = QVBoxLayout(bottomlayoutwidget,0,6,"middlelayout")		
		
		# a first spacer:
		spacer = QSpacerItem(10,10)
		bottomlayout.addItem(spacer)
		
		self.texteditlabel = QLabel(bottomlayoutwidget,"texteditlabel")
		bottomlayout.addWidget(self.texteditlabel)
		
		self.textedit = QTextEdit (bottomlayoutwidget,"Textedit")
		#self.textedit.setMinimumHeight(window_height/5)
		bottomlayout.addWidget(self.textedit)
		
		spacer = QSpacerItem(10,20)
		bottomlayout.addItem(spacer)
		
		self.languageChange()
		
#--------------------------------------------------------------------------------------------------

	def languageChange(self):
		self.texteditlabel.setText(self.__tr("MDclient Shell"))
		
#--------------------------------------------------------------------------------------------------

	def __tr(self,s,c = None):
		return qApp.translate("bottomwidget",s,c)
		
##############################################################
##############################################################

class middlewidget:
	"""
	Class containing the listview, the attributes list, the query-line and the 
	iconview.
	"""
	def __init__(self,splitter1):
		
		self.stop_button_image=QPixmap(images.stop_button)
		self.start_button_image=QPixmap(images.submit_button)
	
		orizzontal_splitter=QSplitter(splitter1)
		
		# Build the listview and add to the layout:
		self.listview = QListView(orizzontal_splitter,"listview")
		self.listview.addColumn(self.__tr("Name"))
		
		# Build a second vertical layoutwidget
		self.middleverticalayoutwidget = QWidget(orizzontal_splitter,"self.middleverticalayoutwidget")
		self.middleverticalayout = QVBoxLayout(self.middleverticalayoutwidget,0,6,"self.middleverticalayout")
		
		# Build a Label for directory attributes and add to the vertical sublayout:
		attrlabel = QLabel(self.middleverticalayoutwidget,"attrlabel")
		self.middleverticalayout.addWidget(attrlabel)
		
		# Build an image, a label, a combobox and a button for selecting entries and add to the vertical sublayout:
		selayoutwidget = QWidget(self.middleverticalayoutwidget,"middlelayoutwidget")
		selayout = QHBoxLayout(selayoutwidget,0,6,"middlelayout")

		# radiobuttons to select exact columns
		self.radio_buttons = selectionradiobuttons(self.middleverticalayoutwidget,self.middleverticalayout)
		
		# Combobox
		self.selcombobox = QComboBox (selayoutwidget,"selcombobox")
		self.selcombobox.setEditable(1)
		selayout.addWidget(self.selcombobox)
		# Send Query Button
		self.go_button = QPushButton(selayoutwidget,"goButton")
		self.go_button.setMinimumWidth(60)
		self.go_button.setMaximumWidth(60)
		self.go_button.setIconSet(QIconSet(self.start_button_image))
		selayout.addWidget(self.go_button)	
		# Stop Button
		self.stop_button = QPushButton(selayoutwidget,"stopButton")
		self.stop_button.setMinimumWidth(60)
		self.stop_button.setMaximumWidth(60)
		self.stop_button.setIconSet(QIconSet(self.stop_button_image))
		selayout.addWidget(self.stop_button)
		# Spacer
		spacer = QSpacerItem(60,0)
		selayout.addItem(spacer)
		
		self.middleverticalayout.addWidget(selayoutwidget)
		
		#middlelayout.addWidget(middlelayoutwidget)
		
		# Build a Iconview for showing the Icons that satisfy the selection
		self.selection_table = QTable(splitter1,"selection_listview")
		self.selection_table.setReadOnly(1)
		
		self.abs_row_index=0
		
		#middlelayout.addWidget(self.selection_table)
		
		self.languageChange(attrlabel)
		
#--------------------------------------------------------------------------------------------------

	def languageChange(self,attrlabel):
		attrlabel.setText(self.__tr("Current Directory Attributes\n"))
		#selabel.setText(self.__tr("Selection"))
		self.go_button.setText("Go")
		self.stop_button.setText("Stop")
		
#--------------------------------------------------------------------------------------------------

	def __tr(self,s,c = None):
		return qApp.translate("pathwidget",s,c)

#############################################################
##############################################################

class pathwidget:
	"""
	Class containing the current path and a label
	"""
	def __init__(self,splitter1,connect_image,disconnect_image,save_image):
		
		#Vertical Layout
		vertlayoutwidget = QWidget(splitter1,"vertlayoutwidget")
		vertlayoutwidget.setFixedHeight(50)
		vertlayout = QVBoxLayout(vertlayoutwidget)
		
		# ButtonLayout
		buttonlayoutwidget = QWidget(vertlayoutwidget,"buttonlayoutwidget")
		buttonlayout = QHBoxLayout(buttonlayoutwidget)
		buttonlayout.setAlignment(1)
		# Connect,Save,Disconnect Buttons
		self.connect_button = QPushButton(buttonlayoutwidget,"connectButton")
		self.connect_button.setFlat(1)
		self.connect_button.setMaximumWidth(25)
		self.connect_button.setMaximumHeight(25)
		self.connect_button.setIconSet(QIconSet(connect_image))
		buttonlayout.addWidget(self.connect_button)
		#
		self.disconnect_button = QPushButton(buttonlayoutwidget,"disconnectButton")
		self.disconnect_button.setFlat(1)
		self.disconnect_button.setMaximumWidth(25)
		self.disconnect_button.setMaximumHeight(25)
		self.disconnect_button.setIconSet(QIconSet(disconnect_image))
		buttonlayout.addWidget(self.disconnect_button)
		#
		self.save_button = QPushButton(buttonlayoutwidget,"saveButton")
		self.save_button.setFlat(1)
		self.save_button.setMaximumWidth(25)
		self.save_button.setMaximumHeight(25)
		self.save_button.setIconSet(QIconSet(save_image))
		buttonlayout.addWidget(self.save_button)	
		
		vertlayout.addWidget(buttonlayoutwidget)
		
		# PathBar layout
		pathbarlayoutwidget = QWidget(vertlayoutwidget,"pathlayoutwidget")
		pathbarlayout = QHBoxLayout(pathbarlayoutwidget)
		
		pathlabel = QLabel(pathbarlayoutwidget,"pathlabel")
		pathbarlayout.addWidget(pathlabel)
		
		self.pathbar = QLineEdit(pathbarlayoutwidget,"PathBar")
		self.pathbar.setReadOnly(1)
		self.pathbar.setText("/")
		
		pathbarlayout.addWidget(self.pathbar)

			
		vertlayout.addWidget(pathbarlayoutwidget)
		
		self.languageChange(pathlabel)

#--------------------------------------------------------------------------------------------------

	def languageChange(self,pathlabel):
		pathlabel.setText(self.__tr("Current Path"))

#--------------------------------------------------------------------------------------------------

	def __tr(self,s,c = None):
		return qApp.translate("pathwidget",s,c)

##############################################################
##############################################################

class selectionradiobuttons:

	def __init__ (self,layoutwidget,layout):
		self.layoutwidget=layoutwidget
		self.layout=layout
		self.list_radiobuttons = []
		self.first_call=1
		self.button_layout_widget= QWidget(self.layoutwidget,"middlelayoutwidget")
		self.button_layout=QHBoxLayout(self.button_layout_widget,0,6,"vlayout")	
		# Gets ready the layout for the button disposition
		n_attr = 45
		n_columns = n_attr/default_n_column_elements+ 1
		self.layout_obj=[]
		for i in range(n_columns):
			pvlayoutwidget = QWidget(self.button_layout_widget,"pvlayoutwidget"+str(i))
			pvlayout=QVBoxLayout(pvlayoutwidget,0,6,"pvlayout"+str(i))
			self.layout_obj.append([pvlayoutwidget,pvlayout])
		
		# Builds the buttons
		self.list_radiobuttons=[]
		for i in range(45):
			layoutindex= i/default_n_column_elements
			vlayoutwidget = self.layout_obj[layoutindex][0]
			vlayout = self.layout_obj[layoutindex][1]
			button = QRadioButton(vlayoutwidget)
			#button.setText(attribute)
			self.list_radiobuttons.append( button )
			vlayout.addWidget (self.list_radiobuttons[i])
		
		for vlayoutwidget_vlayout in self.layout_obj:
			self.button_layout.addWidget(vlayoutwidget_vlayout[0])	
		
		self.layout.insertWidget(1,self.button_layout_widget)
		
		for button in self.list_radiobuttons:
			button.setHidden(1)

#---------------------------------------------------------------------------------------------------------------

	def set_buttons(self,list_of_attributes,n_column_elements=default_n_column_elements):
		counter=0
		for attribute in list_of_attributes:
			self.list_radiobuttons[counter].setText(attribute)
			self.list_radiobuttons[counter].setHidden(0)
			counter+=1
		
		while counter!=45:
			self.list_radiobuttons[counter].setHidden(1)
			counter+=1

		for button in self.list_radiobuttons:
			button.setChecked(1)
			
		self.button_layout_widget.show()
		self.first_call=0
		
		if verbose: print "first call radiobuttons: ",self.first_call