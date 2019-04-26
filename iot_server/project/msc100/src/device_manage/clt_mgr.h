#ifndef __CLT_MGR_H
#define __CLT_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <core/core.h>

#define PORT_CLT    8020

typedef struct _CLT_MGR_OBJECT * CLT_MGR_HANDLE;

CLT_MGR_HANDLE dev_mgr_create(void);
void           dev_mgr_destroy(CLT_MGR_HANDLE handle);
void           dev_mgr_unit_test(void);

#ifdef __cplusplus
}
#endif

#endif // __CLT_MGR_H
