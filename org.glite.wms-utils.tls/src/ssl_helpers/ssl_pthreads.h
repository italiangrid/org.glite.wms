#ifndef _EDG_WL_COMMON_SSL_HELPERS_SSL_PTHREADS_H
#define _EDG_WL_COMMON_SSL_HELPERS_SSL_PTHREADS_H

#ident "$Header$"

#ifdef __cplusplus
extern "C" {
#endif

int edg_wlc_SSLLockingInit(void);
int edg_wlc_SSLLockingCleanup(void);

#ifdef __cplusplus
}
#endif

#endif
