AMGA Test Instructions

In order to run the tests in the AMGA/tests directory you need to have a valid proxy. Use voms-proxy-init with the dteam VO.

If the AMGA-test_functions.py tests fails with the error:
-(6, 'TLS/SSL connection has been closed')-

then it means that your DN is not mapped to the test_user account.
The following command must be executed on the AMGA server:
#> user_subject_add test_user <your DN>

Contact asteriosk@cs.ucy.ac.cy in order to execute the command on the AMGA servers in Cyprus.

Notice that the two tests AMGA-test_ping.py and AMGA-test_statistic.py does not require this mapping.

