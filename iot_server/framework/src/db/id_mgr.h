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

int id_mgr_add_device(ID_MGR_HANDLE handle, char * id);

int id_mgr_add_group_openid(ID_MGR_HANDLE handle, char *id, char * openid);

int id_mgr_del_group_openid(ID_MGR_HANDLE handle, char *id);

int id_mgr_get_uuid_by_group_openid(ID_MGR_HANDLE handle, char * openid, char * buf, int buf_len);

int id_mgr_del_device(ID_MGR_HANDLE handle, char * id);

int id_mgr_id_is_exist(ID_MGR_HANDLE handle, char * id);

int id_mgr_group_openid_is_exist(ID_MGR_HANDLE handle, char * openid);

void id_mgr_destroy(ID_MGR_HANDLE handle);

int   id_mgr_unit_test( void );

#ifdef __cplusplus
}
#endif

#endif
