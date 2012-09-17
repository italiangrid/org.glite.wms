import commands

class _test:
    def __init__(self):
        self.retVal=0
    def info(self):
        return "Implement me !!!"
    def test(self):
        print "Implement me"
    def ret(self):
        print "Implement me!!!"
    def getRetVal(self):
        return self.retVal
    def prepare(self):
        pass
    def clean(self):
        pass
    def get_guid(self):
        return commands.getoutput('uuidgen').split('/n')[0]

class _ntest(_test):
    def __init__(self):
	self.retVal=-1
