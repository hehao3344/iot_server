#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//#include "hash_value.h"
#include "sqlite3.h"
#include "dev_db_mgr.h"

#define MAX_ID_NUMBER   (20000)

//char      max 8000 char
//varcahr   var char max 8000 char
//nchar     unicode, max 4000
//nvarchar  var unicode, max 4000

// "create table client_table(sn integer primary key autoincrement, sn nchar(20))"
// sn: telephone num.

// device: mac address
#define CREATE_CLIENT_STRING            "create table client_table(id integer primary key autoincrement, sn varchar(11), home_group nvarchar(32), email varchar(64), password varchar(32), random char(6), dev_list varchar(256))"
#define CREATE_INDEX_ID_STRING          "create index id_index on client_table(sn)"
#define CREATE_INDEX_DEV_STRING         "create index id_index on client_table(device)" // deleted it?

#define CREATE_DEVICE_STRING            "create table device_table(id integer primary key autoincrement, dev_uuid char(16), on_off int, last_alive int)"
#define CREATE_INDEX_DEV_UUID_STRING    "create index id_index on device_table(dev_uuid)"

typedef struct _DEV_DB_MGR_OBJECT
{
    // use sqlite3 library.
    sqlite3* client_db;         // sn table;
    int max_client;

    sqlite3* device_db;         // sn table;
    int max_device;

    pthread_mutex_t mutex;
} DEV_DB_MGR_OBJECT;

static DEV_DB_MGR_HANDLE instance(void);
static int device_is_exist(DEV_DB_MGR_HANDLE handle,  char * dev_uuid);
static int device_del(DEV_DB_MGR_HANDLE handle, char * dev_uuid);

DEV_DB_MGR_HANDLE dev_db_mgr_create(void)
{
    int result = 0;
    char* errmsg = NULL;

    DEV_DB_MGR_OBJECT *handle = instance();
    if (NULL == handle)
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    //device db create
    debug_print("opening db: %s \n", DEVICE_DB);

    // create event index database.
    result = sqlite3_open(DEVICE_DB, &handle->device_db);
    if (SQLITE_OK != result)
    {
        debug_print("open db: %s failed\n", DEVICE_DB);
        goto create_failed;
    }

    result = sqlite3_exec(handle->device_db, CREATE_DEVICE_STRING, NULL, NULL, &errmsg);
    if (SQLITE_OK != result)
    {
        /* 不判断返回值：因为数据库存在时返回值不是SQLITE_OK */
    }

    result = sqlite3_exec(handle->device_db, CREATE_INDEX_DEV_UUID_STRING, NULL, NULL, &errmsg);
    if (SQLITE_OK != result)
    {
        /* 不判断返回值：因为数据库存在时返回值不是SQLITE_OK */
    }

    handle->max_device = MAX_ID_NUMBER;

    pthread_mutex_init(&handle->mutex, NULL);

    return handle;

create_failed:
    dev_db_mgr_destroy(handle);
    return NULL;
}

int dev_db_mgr_add_device(DEV_DB_MGR_HANDLE handle, char *dev_uuid, int on_off, int last_alive)
{
    int ret    = -1;
    int result = 0;
    char * errmsg = NULL;
    char  sq_cmd[256];

    /* 如果设备不存在则添加否则直接更新 */
    if (0 == device_is_exist(handle, dev_uuid))
    {
        // insert
        memset(sq_cmd, 0, sizeof(sq_cmd));
        snprintf(sq_cmd, sizeof(sq_cmd), "insert into device_table(dev_uuid, on_off, last_alive) values ('%s', '%d', '%d')", dev_uuid, on_off, last_alive);

        pthread_mutex_lock(&handle->mutex);
        result = sqlite3_exec(handle->device_db, sq_cmd, 0, 0, &errmsg);
        pthread_mutex_unlock(&handle->mutex);

        if (SQLITE_OK == result)
        {
            debug_print("insert dev: %s success \n", dev_uuid);
            ret = 0;
        }
        else
        {
            debug_error("insert dev: %s failed \n", dev_uuid);
        }
    }
    else
    {
        // update
        memset(sq_cmd, 0, sizeof(sq_cmd));
        snprintf(sq_cmd, sizeof(sq_cmd), "update device_table set on_off='%d', last_alive='%d' where dev_uuid = '%s'", on_off, last_alive, dev_uuid);

        pthread_mutex_lock(&handle->mutex);
        result = sqlite3_exec(handle->device_db, sq_cmd, 0, 0, &errmsg);
        pthread_mutex_unlock(&handle->mutex);

        if (SQLITE_OK == result)
        {
            debug_print("update dev_uuid:%s success \n", dev_uuid);
            ret = 0;
        }
        else
        {
            debug_print("update dev_uuid:%s failed \n", dev_uuid);
        }
    }

    return ret;
}

// delete db in client/device.
int dev_db_mgr_del_device(DEV_DB_MGR_HANDLE handle, char *dev_uuid)
{
    // del dev_uuid section in client dev_list section.
    if (1 == device_is_exist(handle, dev_uuid))
    {
        // delete device in device db.
        if (0 == device_del(handle, dev_uuid))
        {
            debug_print("delete %s success \n", dev_uuid);
        }
    }
    else
    {
        debug_print("no dev_uuid  %s ?\n", dev_uuid);
    }

    return 0;
}

int dev_db_mgr_device_get_param(DEV_DB_MGR_HANDLE handle, char * dev_uuid, DEV_PARAM * param)
{
    int ret = -1;
    int i, j;
    int result = 0;
    char* errmsg = NULL;

    char sq_cmd[256];
    char **dbResult;
    int nRow, nColumn;

    memset(sq_cmd, 0, sizeof(sq_cmd));
    snprintf(sq_cmd, sizeof(sq_cmd), "select * from device_table where dev_uuid = '%s'", dev_uuid);

    result = sqlite3_get_table(handle->device_db, sq_cmd, &dbResult, &nRow, &nColumn, &errmsg);
    if (SQLITE_OK == result)
    {
        for(i=0; i<nRow; i++)
        {
            for(j=0; j<nColumn; j++)
            {
                if (0 == strcmp(dbResult[j], "on_off"))
                {
                     param->on_off = atoi(dbResult[nColumn + j]);
                }
                else if (0 == strcmp(dbResult[j], "last_alive"))
                {
                    param->on_off = atoi(dbResult[nColumn + j]);
                }
            }
        }
        if ((nRow > 0) && (nColumn > 0))
        {
            ret = 0;
        }
    }

    sqlite3_free_table(dbResult);

    return ret;
}

void dev_db_mgr_destroy(DEV_DB_MGR_HANDLE handle)
{
    if (NULL == handle)
    {
        return;
    }

    pthread_mutex_destroy(&handle->mutex);
    sqlite3_close(handle->client_db);
    sqlite3_close(handle->device_db);
    free(handle);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// static function
/////////////////////////////////////////////////////////////////////////////////////////////
static DEV_DB_MGR_OBJECT* instance(void)
{
    static DEV_DB_MGR_OBJECT* handle = NULL;
    if (NULL == handle)
    {
        handle = (DEV_DB_MGR_OBJECT *)calloc(1, sizeof(DEV_DB_MGR_OBJECT));
    }

    return handle;
}

static int device_is_exist(DEV_DB_MGR_HANDLE handle, char * dev_uuid)
{
    int ret = 0;
    int result = 0;
    char* errmsg = NULL;

    char sq_cmd[128];
    char **dbResult;
    int nRow, nColumn;

    memset(sq_cmd, 0, sizeof(sq_cmd));
    sprintf(sq_cmd, "select dev_uuid from device_table where dev_uuid = '%s'", dev_uuid);
    result = sqlite3_get_table(handle->device_db, sq_cmd, &dbResult, &nRow, &nColumn, &errmsg);
    if (SQLITE_OK == result)
    {
        if ((nRow > 0) && (nColumn > 0))
        {
            ret = 1;
        }
    }
    sqlite3_free_table(dbResult);

    return ret;
}

static int device_del(DEV_DB_MGR_HANDLE handle, char * dev_uuid)
{
    char sq_cmd[128];
    int result = 0;
    sqlite3_stmt *stmt = NULL;
    const char * errorCode = NULL;

    // delete the first item here.
    sqlite3_exec(handle->device_db, "begin", 0, 0, 0);

    memset(sq_cmd, 0, sizeof(sq_cmd));
    sprintf(sq_cmd, "delete from device_table  where dev_uuid = '%s'", dev_uuid);

    pthread_mutex_lock(&handle->mutex);
    result = sqlite3_prepare(handle->device_db, sq_cmd, -1, &stmt, &errorCode);
    if (result != SQLITE_OK)
    {
        debug_error("result Code:%d error message:%s", result, errorCode);
        sqlite3_finalize(stmt);
        pthread_mutex_unlock(&handle->mutex);
        return -1;
    }

    result = sqlite3_step(stmt);
    if (result!= SQLITE_DONE)
    {
        debug_print("delete failed! Result Code:%d  ErrorMessage:%s", result, errorCode);
        sqlite3_finalize(stmt);
        pthread_mutex_unlock(&handle->mutex);
        return -1;
    }
    pthread_mutex_unlock(&handle->mutex);

    sqlite3_finalize(stmt);
    sqlite3_exec(handle->device_db, "commit", 0, 0, 0);

    return 0;
}

int dev_db_mgr_unit_test(void)
{
    int ret = -1;
    DEV_DB_MGR_HANDLE handle = dev_db_mgr_create();
    if (NULL == handle)
    {
        return -1;
    }
    char * dev_uuid = "0011220398789983";
    ret = dev_db_mgr_add_device(handle, dev_uuid, 1, 2398478);
    if (0 != ret)
    {
        debug_error("dev_db_mgr_add_device failed \n");
        return -1;
    }

#if 0
    DEV_PARAM dev_param;
    memset(&dev_param, 0, sizeof(DEV_PARAM));
    ret = dev_db_mgr_device_get_param(handle, dev_uuid,  &dev_param);
    if (0 != ret)
    {
        debug_error("dev_db_mgr_add_device failed \n");
        return -1;
    }
    debug_print("get uuid %s on_off %d alive %d \n", dev_param.dev_uuid, dev_param.on_off, dev_param.last_alive);

    dev_db_mgr_del_device(handle, dev_uuid);
    dev_db_mgr_del_device(handle, dev_uuid);

    memset(&dev_param, 0, sizeof(DEV_PARAM));
    ret = dev_db_mgr_device_get_param(handle, dev_uuid,  &dev_param);
    if (0 != ret)
    {
        debug_error("dev_db_mgr_device_get_param failed \n");
        return -1;
    }
    debug_print("get uuid %s on_off %d alive %d \n", dev_param.dev_uuid, dev_param.on_off, dev_param.last_alive);
#endif

    dev_db_mgr_destroy(handle);

    return 0;
}
