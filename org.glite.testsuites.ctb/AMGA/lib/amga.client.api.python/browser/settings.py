#This file contains all the global variables necessary for setting up the gui and for
#the thread management.


import threading
import os

verbose=True

# Global Parameters ---------------------------------------------------------------------------
# Server Settings - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
#server_name = 'lxarda01.cern.ch'
#port_number =8822
#user='root'
# WIndow's Size - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
window_width  = 2000
window_height = 2000
# History file - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
history_file_name = os.path.expanduser("~")+"/.selection_history"
# No of recorded commands  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
number_of_recorded_commands= 50
# Attributes separator - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
attributes_separator=" <-> "
# Condition for Threads - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
workavailable = threading.Condition()
# Mdclient Prompt - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
mdclient_prompt = "Query >>"
# Default combobox Item
default_selcomboboxitem="Make a selection"
# Md client command
mdclient_command="/opt/glite/bin/mdclient"
# Default number of radiobuttons per column
default_n_column_elements=8
# Set the number of entries added to the table per periodic call
n_table_items_per_periodic_call=50
# Set the maximum number of entries put in the listview 
max_read_entries_number= 5000
#--------------------------------------------------------------------------------------------------------