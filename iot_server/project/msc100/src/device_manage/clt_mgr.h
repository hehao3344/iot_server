#ifndef __CLT_MGR_H
#define __CLT_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <core/core.h>

#define PORT_CLT    8020

typedef struct _CLT_MGR_OBJECT * CLT_MGR_HANDLE;

CLT_MGR_HANDLE clt_mgr_create(void);
void           clt_mgr_destroy(CLT_MGR_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif // __CLT_MGR_H
