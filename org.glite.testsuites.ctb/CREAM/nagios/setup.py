#!/usr/bin/env python

from distutils.core import setup

comp_version = '1.0.0'

probe_data = [
              'src/CREAMCEDJS-probe',
              'src/CREAMCE-probe',
              'src/CREAMDJS-jdl.template',
              'src/CREAM-jdl.template'
             ]
setup(
      name='emi-cream-nagios',
      version=comp_version,
      description='Nagios probe for the EMI CREAM and WN services',
      license='Apache Software License',
      packages=['creammetrics'],
      package_dir = {'': 'src'},
      data_files=[
                  ('etc/gridmon', ['etc/emi.nagios-jobmngt.conf']),
                  ('usr/libexec/grid-monitoring/probes/emi.cream', probe_data),
                  ('usr/share/doc/emi-cream-nagios-' + comp_version, ['CHANGES', 'README'])
                 ]
     )

