#ifndef __OPENID_H
#define __OPENID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <core/core.h>

#define PORT_OPENID    8030

typedef struct _OPENID_OBJECT * OPENID_HANDLE;

OPENID_HANDLE  openid_create(void);
void           openid_destroy(OPENID_HANDLE handle);
void           openid_unit_test(void);

#ifdef __cplusplus
}
#endif

#endif // __OPENID_H