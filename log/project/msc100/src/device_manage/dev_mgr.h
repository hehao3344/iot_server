#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <core/core.h>

#define PORT_DEV    8010
#define PORT_CLT    8020

typedef struct _DEV_MGR_OBJECT * DEV_MGR_HANDLE;

DEV_MGR_HANDLE dev_mgr_create(void);
void           dev_mgr_destroy(DEV_MGR_HANDLE handle);
void           dev_mgr_unit_test(void);

#ifdef __cplusplus
}
#endif

#endif // __TCP_SERVER_H
