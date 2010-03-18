

gcc -I/opt/lcg/include/dpm -L/opt/globus/lib -L/opt/lcg/lib -ldpm -ldl -lglobus_openssl_gcc32 -lm utils.c DPNS_grpmap_remote.c -o DPNS_grpmap_remote
if [ $? -ne 0 ]; then
   echo "Compilation error. Abort"
   exit 1
fi

scp DPNS_grpmap_remote root@head32.cern.ch:/tmp >/dev/null

ssh root@head32.cern.ch "/tmp/DPNS_grpmap_remote head32.cern.ch /dpm/cern.ch/home/email/TESTS"


