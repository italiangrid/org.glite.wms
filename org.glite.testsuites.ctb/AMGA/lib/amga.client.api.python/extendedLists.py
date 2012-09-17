################################################################################
# Ganga Project. http://cern.ch/ganga
#
# $Id$
################################################################################
import os
import types
import time

from diskutils import *

class Block(object):
    def __init__(self, entries = None, name = None):
        if entries == None:
            entries = []
        self.entries = entries
        self.name = name
        self.modified = 0

    def genericName(self):
        if self.name:
            return self.name.split('_')[0]


class BlockedList(types.ListType):
    blocksize  = 100
    block_prefix = 'BLOCK-'
    
    def __init__(self, ll = None):
        if ll == None:
            ll = []
        super(BlockedList, self).__init__(ll)
        self.blocks = []
        self.blkrefs = []
        thrshd = 0
        for i in range(len(self)):
            if i == thrshd:
                thrshd += self.blocksize
                blk = Block()
                self.blocks.append(blk)
            self.blkrefs.append(blk)
            blk.entries.append(self[i])
    
    def __delitem__(self, i):
        blk = self.blkrefs[i]
        ofs = self._blkOffset(blk)
        del blk.entries[i - ofs]
        del self.blkrefs[i]
        super(BlockedList, self).__delitem__(i)
        blk.modified = 1

    def __setitem__(self, i, item):
        super(BlockedList, self).__setitem__(i, item)
        blk = self.blkrefs[i]
        ofs = self._blkOffset(blk)
        blk.entries[i - ofs] = item
        blk.modified = 1

    def _blkOffset(self, blk):
        ofs = 0
        for b in self.blocks:
            if b is blk:
                break
            ofs += len(b.entries)
        return ofs
        
    def _getCachedBlockNames(self):
        dd = {}
        for blk in self.blocks:
            generic_name = blk.genericName()
            if generic_name != None:
                dd[generic_name] = blk
        return dd

    def _sorter(self, x, y):
        x, y = map(lambda f: int(f.split('-')[1]), (x, y))
        if x < y:
            return -1
        elif x == y:
            return 0
        elif x > y:
            return 1

    def _getStoredBlockNames(self, dirname):
        names = []
        for fn in listFiles(dirname):
            if fn.startswith(self.block_prefix):
                ff = fn.split('_')
                if len(ff) == 2:
                    if ff[1] != '':
                        fn = ff[0]
                        if fn not in names:
                            names.append(fn)
        names.sort(self._sorter)
        return names       

    def append(self, item):
        super(BlockedList, self).append(item)
        try:
            if self.blkrefs:
                blk = self.blkrefs[-1]
                if len(blk.entries) < self.blocksize:
                    return
            blk = Block()
            self.blocks.append(blk)
            return
        finally:
            blk.entries.append(item)
            self.blkrefs.append(blk)
            blk.modified = 1
            
    def load(self, dirname):
        entries = []
        blocks = []
        blkrefs = []
        bns = self._getStoredBlockNames(dirname)
        ndict = self._getCachedBlockNames()
        for bn in bns:
            try:
                last = getLast(os.path.join(dirname, bn))
            except OSError, e:
                continue
            lfn  = os.path.basename(last)
            try:
                if bn in ndict:
                    blk = ndict[bn]
                    if blk.name == lfn:
                        blk.modified = 0
                        continue
                blk = Block(name = lfn)
                blk.entries = read(last)
            finally:
                blocks.append(blk)
                entries.extend(blk.entries)
                blkrefs.extend([blk]*len(blk.entries))
        self[:] = entries
        self.blocks = blocks
        self.blkrefs = blkrefs

    def save(self, dirname):
        bns = self._getStoredBlockNames(dirname)
        if len(bns) > 0:
            bni = int(bns[-1].split('_')[0].split('-')[1])+ 1
        else:
            bni = 0
        for blk in self.blocks:
            generic_name = blk.genericName()
            if not generic_name:
                generic_name = self.block_prefix + str(bni)
                bni += 1
            fn = os.path.join(dirname, generic_name)
            if len(blk.entries) > 0:
                if blk.modified:
                    blk.name = os.path.basename(write(blk.entries, fn))
            else:
                if generic_name in bns:
                    remove(fn)                 

    def mark(self, i):
        blk = self.blkrefs[i]
        blk.modified = 1
        

class Attributes(BlockedList):
    blocksize  = 20
    block_prefix = 'Attr-'
    

class Entries(BlockedList):
    blocksize  = 100
    block_prefix = 'Entr-'        
