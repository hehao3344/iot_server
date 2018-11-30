#ifndef __DEVICE_MGR_H
#define __DEVICE_MGR_H

#include <core/core.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_ID_LEN  36  /* ���ID���� */

typedef struct _DEV_PARAM
{
    int8 name[32];
    int8 version[8];
    int8 on_off[4];         // on off
    int8 pm25[8];
    int8 filter_time[8];
} DEV_PARAM;

typedef struct GroupMgrObject *  GROUP_MGR_HANDLE;

GROUP_MGR_HANDLE group_mgr_create( void );
boolean group_mgr_client_get_random( GROUP_MGR_HANDLE handle, int8 * sn, int8 * random_out );
int32   group_mgr_client_register( GROUP_MGR_HANDLE handle, int8 * sn, int8 * home_group,
                                   int8 * email, int8 *password, int8 * random );
boolean group_mgr_client_get_password( GROUP_MGR_HANDLE handle, int8 *sn, int8 *password_out );
int32   group_mgr_add_device( GROUP_MGR_HANDLE handle, int8 *sn, int8 *mac, int8 *dev_name, int8 *version );
int32   group_mgr_del_device( GROUP_MGR_HANDLE handle, int8 *sn, int8 *mac );
int32   group_mgr_client_get_device_mac( GROUP_MGR_HANDLE handle, int8 *sn, int8 *mac[] );
boolean group_mgr_device_get_param( GROUP_MGR_HANDLE handle, int8 *mac, DeviceParam *param );
void    group_mgr_destroy( GROUP_MGR_HANDLE handle );

int32   group_mgr_unit_test( void );

#ifdef __cplusplus
}
#endif

#endif
