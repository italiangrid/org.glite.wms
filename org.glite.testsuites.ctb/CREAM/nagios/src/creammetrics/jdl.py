##############################################################################
#
# NAME:        jdl.py
#
# FACILITY:    SAM (Service Availability Monitoring)
#
# COPYRIGHT:
#         Copyright (c) 2009, Members of the EGEE Collaboration.
#         http://www.eu-egee.org/partners/
#         Licensed under the Apache License, Version 2.0.
#         http://www.apache.org/licenses/LICENSE-2.0
#         This software is provided "as is", without warranties
#         or conditions of any kind, either express or implied.
#
# DESCRIPTION:
#
#         JDL related classes.
#
# AUTHORS:     Konstantin Skaburskas, CERN
#
# CREATED:     22-Mar-2010
#
# NOTES:
#
# MODIFIED:
#
##############################################################################

"""
JDL related classes.

JDL class for templated substitution.

Konstantin Skaburskas <konstantin.skaburskas@cern.ch>, CERN
SAM (Service Availability Monitoring)
"""

from gridmon.template import TemplatedFile
from gridmon.metricoutput import OutputHandlerSingleton

class JDLTemplate(OutputHandlerSingleton):
    '''Provides templated JDL substitutions.
    '''
    def __init__(self, file_templ, file, mappings={}):
        OutputHandlerSingleton.__init__(self)
        
        self.__tmpl = TemplatedFile(file_templ, file, mappings)      

    def build_save(self):
        """Substitute and save. 
        """
        tmpl = self.__tmpl
        self.printdvm('# Template file: %s' % tmpl.file_templ)
        
        tmpl.load()
        self.printdvm('# Template:\n%s' % tmpl.template)
        
        self.printdvm('# Mappings for template substitutions:\n%s' % 
            '\n'.join(['%s : %s'%(k,v) for k,v in tmpl.mappings.items()]))
        tmpl.subst()
        self.printdvm('# Resulting substitutions:\n%s' % tmpl.substitution)
        
        self.printdvm('# Saving to %s ... ' % tmpl.file, cr=False)
        tmpl.save()
        self.printdvm('done.')
