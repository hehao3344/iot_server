#ifndef __DEVICE_MGR_H
#define __DEVICE_MGR_H

#include <core/core.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CLIENT_DB               "client.db"
#define DEVICE_DB               "device.db"

#define MAX_ID_LEN      16      /* 最大ID长度 */

typedef struct _DEV_PARAM
{
    char dev_uuid[MAX_ID_LEN];
    char on_off;
    int  last_alive;            /* 最后一次心跳时间 */
} DEV_PARAM;

typedef struct _DEV_DB_MGR_OBJECT *  DEV_DB_MGR_HANDLE;

DEV_DB_MGR_HANDLE dev_db_mgr_create(void);

int dev_db_mgr_add_device(DEV_DB_MGR_HANDLE handle, char *dev_uuid, int on_off, int last_alive);

int dev_db_mgr_del_device(DEV_DB_MGR_HANDLE handle, char *dev_uuid);

int dev_db_mgr_device_get_param(DEV_DB_MGR_HANDLE handle, char * dev_uuid, DEV_PARAM * param);

void dev_db_mgr_destroy(DEV_DB_MGR_HANDLE handle);

int   dev_db_mgr_unit_test( void );

#ifdef __cplusplus
}
#endif

#endif
