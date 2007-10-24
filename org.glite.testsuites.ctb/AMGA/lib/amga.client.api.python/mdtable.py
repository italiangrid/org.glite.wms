#
# $Id$
#
import os
import time
from extendedLists import Entries, Attributes
from diskutils import RLock
from mdinterface import CommandException

class EmptyTableException(Exception):
      def __init__(self):
          self.raised = True


class MDTable:
    def __init__(self, dirname, attributes = None, entries = None):
        """dirname is system path to the table files"""
        self.attributeDict = {}
        self.typeDict = {}
        self.dirname = dirname
        if attributes == None:
            attributes = Attributes()
        if entries == None:
            entries = Entries()
        self.attributes = attributes
        self.entries = entries
        self.timestamp = time.time()
        self.currentRow = 0
        if not len(entries):
            self.currentRow = -1
        self.update()
        self._lock = RLock(os.path.join(dirname, 'LOCK'))


    # Legathy routine to support old table format, will always write new format!
    def __readEntries(self):
        attributes = []
        entries = []
        name = os.path.normpath(self.dirname + '/ENTRIES')
        try:
            f=open(name, 'r')
        except IOError, e:
            return attributes, entries
        line = f.readline()
        if not line:
            f.close
            return attributes, entries
        line = line.rstrip('\n')
        if line :
            attributes = map(lambda x: x.split(" ", 1), line.split('\0'))
        i = 0
        while 1:
            line = f.readline()
            line = line.rstrip('\n')
            if not line:
                break
            fields = line.split('\0')
            entries.append(fields)
            i=i+1
        f.close()
        return attributes, entries


    def lock(self):
        return self._lock.acquire()
    
        
    def unlock(self):
        self._lock.release()


    def load(self):
        if os.path.isdir(self.dirname):
            if self.lock():
                try:
                    self.attributes.load(self.dirname)
                    self.entries.load(self.dirname)
                    if len(self.attributes) == 0:
                        # try old table format
                        attributes, entries = self.__readEntries()
                        self.attributes = Attributes(attributes)
                        self.entries    = Entries(entries)
                        # flag that we want save all in a new format
                        for lst in [self.attributes, self.entries]:
                            for i in range(len(lst)):
                                lst.mark(i)
                    # consistency check
                    entr_len = len(self.attributes) + 1
                    for entry in self.entries:
                        this_len = len(entry)
                        if this_len > entr_len:
                            del entry[entr_len:]
                        elif this_len < entr_len:
                            entry.extend(['']*(entr_len - this_len))
                    self.timestamp = time.time()
                    self.update()
                finally:
                    self.unlock()
            else:
                raise CommandException(9, 'Could not acquire table lock %s' % self.dirname)
        

    def save(self):
#        mkdir(self.dirname)
        if self.lock():
            try:
                self.update()
                self.attributes.save(self.dirname)
                self.entries.save(self.dirname)
            finally:
                self.unlock()
        else:
            raise CommandException(9, 'Could not acquire table lock %s' % self.dirname)



    def update(self):
        for i in range(0, len(self.attributes)):
            a, t=self.attributes[i] # Was a, t=self.attributes[i].split(' ', 1)
            self.attributeDict[a]=i
            self.typeDict[a] = t
        self.initRowPointer()

    def initRowPointer(self):
        self.currentRow = 0
        if not len(self.entries):
            self.currentRow = -1
