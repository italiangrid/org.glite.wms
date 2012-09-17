################################################################################
# Ganga Project. http://cern.ch/ganga
#
# $Id$
################################################################################
import sys
import os
import types
import time
import errno
import atexit
import random


if time.time() < time.time():
    timestampMethod = time.time
elif time.clock() < time.clock():
    timestampMethod = time.clock
else:
    raise OSError("Can not find an appropriate timestamp function in the time module")

#---------------------------------------------------------------------------
def newGuid(value = None):
    """newGUID(value = None) --> guid value - any python object"""
    tt = time.gmtime()[0:6] + (random.randint(0,9),
                               random.randint(0,9),
                               random.randint(0,9),
                               id(value))
    return '_' + (''.join(map(str, tt))).replace('-','')


def createLock(lockfile, guid):
    tries = 0
    while True:
        tries = tries +1
        try:
            guid += '\0' #mark the end of string
            fd = os.open(lockfile, os.O_WRONLY | os.O_CREAT | os.O_EXCL)
            try:
                guidlen = os.write(fd, guid)
            finally:
                os.close(fd)
            fd = os.open(lockfile, os.O_RDONLY)
            try:
                f_guid = os.read(fd, guidlen)
            finally:
                os.close(fd)
            if f_guid != guid:
                raise OSError("Guids mismatch in the lock file")
        except OSError, e:
            if tries > 200:
                return False
            time.sleep(0.05)
        else:
            return True


def removeLock(lockfile):
    try:
        os.remove(lockfile)
    except OSError, e:
        if e[0] != errno.ENOENT:
            return False
    return True

    

class RLock(object):
    def __init__(self, lockfile = 'Lock'):
        self.lockfile = lockfile
        self.guid = newGuid(self)
        self.counter = 0
        atexit.register(self.remove)

    def acquire(self):
        if self.counter == 0:
            if not createLock(self.lockfile, self.guid):
                return False
        self.counter += 1
        return True
    
    def release(self):
        if self.counter > 0:
            if self.counter == 1:
                if not removeLock(self.lockfile):
                    return
            self.counter -= 1

    def remove(self):
        if self.counter == 0:
            try:
                f = file(self.lockfile, 'r')
                try:
                    f_guid = f.read()
                finally:
                    f.close()
            except:
                return
            if f_guid.endswith('\0'): #lock file was correctly written
                if f_guid.rstrip('\0') != self.guid: # by another RLock
                    return
        removeLock(self.lockfile)
        self.counter = 0
          

class Filterer(object):
    def __init__(self, fn, tmp = False):
        self.fn = fn
        self.tmp = tmp
    def __call__(self, fn):
        ff = fn.split('_')
        fflen = len(ff)
        if fflen > 1:
            if ff[0] == self.fn:
                if self.tmp:
                    if fflen == 3:
                        if ff[2] == '':
                            return True
                else:
                    if fflen == 2:
                        if ff[1] != '':
                            return True
        return False
                    

def listFiles(dirname):
    files = []
    for e in os.listdir(dirname):
        if os.path.isfile(os.path.join(dirname, e)):
            files.append(e)
    return files


def getTimeStamp(fn):
    # fn -- precise
    fn = os.path.basename(fn)
    ff = fn.split('_')
    if len(ff) > 1:
        return float(ff[1])
    return 0


def sorter(x, y):
    x, y = map(getTimeStamp, (x,y))
    if x < y:
        return -1
    elif x == y:
        return 0
    elif x > y:
        return 1


def getHistory(fn, tmp = False):
    # fn -- generic
    dirname, basename = os.path.split(os.path.abspath(fn))
    ffs = filter(Filterer(basename, tmp), listFiles(dirname))
    ffs.sort(sorter)
    return (dirname, ffs)


def getLast(fn):
    # fn -- generic
    dirname, ffs = getHistory(fn)
    ffs.reverse()
    for hf in ffs:  
        hfn = os.path.join(dirname, hf)
        tmp_fn = hfn + '_'
        if not os.path.isfile(tmp_fn):
            break
    else:
        raise OSError("Can not find file %s" % fn)
    return hfn


def read(fn):
    # fn -- precise
    entries = []
    f = open(fn, 'r')
    try:
        while 1:
            line = f.readline()
            line = line.rstrip('\n')
            if not line:
                break
            fields = line.split('\0')
            entries.append(fields)
        return entries
    finally:
        f.close()
        

def readLast(fn):
    # fn -- generic
    return read(getLast(fn))

    
def move(tmp_fn, fn):
    # tmp_fn, fn -- precise
    if os.path.isfile(fn):
        raise OSError("File exists %s" % fn) 
    os.rename(tmp_fn, fn)
    # check that file was moved
    tries = 0
    while True:
        tries = tries +1
        if os.path.isfile(tmp_fn) or not os.path.isfile(fn):
            if tries > 200:
                raise OSError("Can not rename file %s" % tmp_fn)
            time.sleep(0.05)
        else:
            return


def remove(fn):
    # fn -- generic
    dirname, ffs = getHistory(fn, tmp = False)
    for hf in ffs:  
        fn = os.path.join(dirname, hf)
        os.remove(fn)
        # check that file was removed
        tries = 0
        while True:
            tries = tries +1
            if os.path.isfile(fn):
                if tries > 200:
                    raise OSError("Can not remove file %s" % fn)
                time.sleep(0.05)
            else:
                return    
    

def write(entries, fn):
    # fn -- generic
    dirname, ffs = getHistory(fn, tmp = False)
    dirname, hfs = getHistory(fn, tmp = True)
    timestamp = '%.7f' % timestampMethod()
    #timestamp = '%.7f' % time.time()
    fn = '_'.join([fn, timestamp])
    tmp_fn = fn + '_'
    f = open(tmp_fn, 'w')
    try:
        for entry in entries:
            if not len(entry):
                continue
            line=''
            for field in entry:
                if len(line):
                    line += '\0'
                line += str(field)
            f.write(line + '\n')
    finally:
        f.close()
    move(tmp_fn, fn)
    for fls in [ffs, hfs]:
        for hf in fls:
            hf = os.path.join(dirname, hf)
            try:
                os.remove(hf)
            except OSError:
                pass
    return fn
    
