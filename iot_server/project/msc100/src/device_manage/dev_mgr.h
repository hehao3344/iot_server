#ifndef __DEV_MGR_H
#define __DEV_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <core/core.h>

#include "dev_param.h"

#define PORT_DEV    8010

typedef struct _DEV_MGR_OBJECT * DEV_MGR_HANDLE;

DEV_MGR_HANDLE   dev_mgr_create(void);
DEV_PARAM_HANDLE dev_mgr_get_dev_param_handle(DEV_MGR_HANDLE handle);
void             dev_mgr_destroy(DEV_MGR_HANDLE handle);
void             dev_mgr_unit_test(void);

#ifdef __cplusplus
}
#endif

#endif // __DEV_MGR_H
