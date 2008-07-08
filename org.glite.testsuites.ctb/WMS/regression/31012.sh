#!/bin/bash

# Check the version returned by the server and the ones of the command.
# They must be the same number

glite-wms-job-submit --debug --version --config etc/wmp_devel19.conf  | grep " Version" | sed -e "s/.*: //"
