#!/usr/bin/env python

from distutils.core import setup

comp_version = '1.0.0'

probe_data = [
              'src/CREAMCEDJS-probe',
              'src/CREAMCE-probe'
             ]

template_data = [
                 'src/CREAMDJS-jdl.template',
                 'src/CREAM-jdl.template'
                ]
pkg_list = [
            'creammetrics',
            'dirq',
            'metric',
            'mig',
            'msgdirq',
            'stomp'
           ]

pkg_dir_dict = {
                '': 'src',
                'dirq': 'src/wnjob/nagios.d/lib/python2.3/site-packages/dirq',
                'metric': 'src/wnjob/nagios.d/lib/python2.3/site-packages/metric',
                'mig': 'src/wnjob/nagios.d/lib/python2.3/site-packages/mig',
                'msgdirq': 'src/wnjob/nagios.d/lib/python2.3/site-packages/msgdirq',
                'stomp': 'src/wnjob/nagios.d/lib/python2.3/site-packages/stomp'
               }
               
setup(
      name='emi-cream-nagios',
      version=comp_version,
      description='Nagios probe for the EMI CREAM and WN services',
      license='Apache Software License',
      packages=pkg_list,
      package_dir = pkg_dir_dict,
      data_files=[
                  ('etc/gridmon', ['etc/emi.nagios-jobmngt.conf']),
                  ('usr/libexec/grid-monitoring/probes/emi.cream', probe_data),
                  ('usr/share/emi-cream-nagios', template_data),
                  ('usr/share/doc/emi-cream-nagios-' + comp_version, ['README'])
                 ]
     )

