#ifndef GLITE_WMS_WMPROXY_WMPROXY_H
#define GLITE_WMS_WMPROXY_WMPROXY_H

#define MAX_THREAD_NUM (8) // maximum thread number
#define BACKLOG (8) // socket listener backlog
#define SERVER_ADDRESS "10.3.1.43" // service socket address
#define SERVER_PORT (8080) // service socket port
#define LB_ADDRESS "gundam.cnaf.infn.it" // logging&bookkeeping server address
//#define LB_ADDRESS "aither.zcu.cz" // logging&bookkeeping server address
#define LB_PORT (9000) // logging&bookkeeping server port

#endif // GLITE_WMS_WMPROXY_WMPROXY_H
