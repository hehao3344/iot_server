#ifndef __ID_MGR_H
#define __ID_MGR_H

#include <core/core.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ID_DB           "id.db"

typedef struct _ID_MGR_OBJECT *  ID_MGR_HANDLE;

ID_MGR_HANDLE id_mgr_create(void);

int id_mgr_add_device(ID_MGR_HANDLE handle, char *id, char * product_key, char *dev_secret);

int id_mgr_add_group_openid(ID_MGR_HANDLE handle, char *id, char * openid);

int id_mgr_update_dev_name(ID_MGR_HANDLE handle, char *id, char * dev_name);

int id_mgr_update_product_key(ID_MGR_HANDLE handle, char *id, char * product_key);

int id_mgr_update_dev_secret(ID_MGR_HANDLE handle, char *id, char * dev_secret);

int id_mgr_update_online_time(ID_MGR_HANDLE handle, char *id, char * last_online_time);

int id_mgr_update_offline_time(ID_MGR_HANDLE handle, char *id, char * last_offline_time);

int id_mgr_del_group_openid(ID_MGR_HANDLE handle, char *id, char * gopenid);

int id_mgr_get_uuid_by_group_openid(ID_MGR_HANDLE handle, char * openid, char * buf, int buf_len);

int id_mgr_get_product_key_by_group_openid(ID_MGR_HANDLE handle, char * openid, char * buf, int buf_len);

int id_mgr_get_dev_secret_by_group_openid(ID_MGR_HANDLE handle, char * openid, char * buf, int buf_len);

int id_mgr_del_device(ID_MGR_HANDLE handle, char * id);

int id_mgr_id_is_exist(ID_MGR_HANDLE handle, char * id);

int id_mgr_group_openid_is_exist(ID_MGR_HANDLE handle, char * openid);

int id_mgr_get_uuid_by_index(ID_MGR_HANDLE handle, int index, char * buf, int buf_len);

typedef void (* got_callback)(void * arg, int index, char * dev_uuid, char * product_key, char * dev_secret, char * openid, char * dev_name, char * online_time, char * offline_time, int online);

int id_mgr_get_uuid(ID_MGR_HANDLE handle, got_callback fn_cb, void * arg);

void id_mgr_destroy(ID_MGR_HANDLE handle);

int   id_mgr_unit_test( void );

#ifdef __cplusplus
}
#endif

#endif
