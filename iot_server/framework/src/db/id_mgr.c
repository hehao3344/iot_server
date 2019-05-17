#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "sqlite3.h"
#include "id_mgr.h"

#define MAX_ID_NUMBER                   (20000)

#define CREATE_ID_STRING                "create table id_table(id integer primary key autoincrement, dev_uuid char(16), gopenid char(32), openid1 char(32), openid2 char(32), openid3 char(32))"
#define CREATE_INDEX_ID_STRING          "create index id_index on id_table(dev_uuid)"

typedef struct _ID_MGR_OBJECT
{
    sqlite3* id_db;                     // sn table;
    int max_id_count;

    pthread_mutex_t mutex;
} ID_MGR_OBJECT;

static ID_MGR_HANDLE instance(void);
static int device_is_exist(ID_MGR_HANDLE handle,  char * id);
static int device_del(ID_MGR_HANDLE handle, char * id);
static int group_openid_is_exist(ID_MGR_HANDLE handle, char * openid);

ID_MGR_HANDLE id_mgr_create(void)
{
    int result = 0;
    char * errmsg = NULL;

    ID_MGR_OBJECT *handle = instance();
    if (NULL == handle)
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    //device db create
    debug_print("opening db: %s \n", ID_DB);

    // create event index database.
    result = sqlite3_open(ID_DB, &handle->id_db);
    if (SQLITE_OK != result)
    {
        debug_print("open db: %s failed\n", ID_DB);
        goto create_failed;
    }

    result = sqlite3_exec(handle->id_db, CREATE_ID_STRING, NULL, NULL, &errmsg);
    if (SQLITE_OK != result)
    {
        /* 不判断返回值：因为数据库存在时返回值不是SQLITE_OK */
    }

    result = sqlite3_exec(handle->id_db, CREATE_INDEX_ID_STRING, NULL, NULL, &errmsg);
    if (SQLITE_OK != result)
    {
        /* 不判断返回值：因为数据库存在时返回值不是SQLITE_OK */
    }

    handle->max_id_count = MAX_ID_NUMBER;

    pthread_mutex_init(&handle->mutex, NULL);

    return handle;

create_failed:
    id_mgr_destroy(handle);
    return NULL;
}

int id_mgr_add_device(ID_MGR_HANDLE handle, char *id)
{
    int ret    = -1;
    int result = 0;
    char * errmsg = NULL;
    char  sq_cmd[256];

    /* 如果设备不存在则添加否则直接更新 */
    if (0 == device_is_exist(handle, id))
    {
        // insert
        memset(sq_cmd, 0, sizeof(sq_cmd));
        snprintf(sq_cmd, sizeof(sq_cmd), "insert into id_table(dev_uuid) values ('%s')", id);

        pthread_mutex_lock(&handle->mutex);
        result = sqlite3_exec(handle->id_db, sq_cmd, 0, 0, &errmsg);
        pthread_mutex_unlock(&handle->mutex);

        if (SQLITE_OK == result)
        {
            debug_print("insert dev: %s success \n", id);
            ret = 0;
        }
        else
        {
            debug_error("insert dev: %s failed \n", id);
        }
    }
    else
    {
        debug_print("dev: %s exist already \n", id);
        ret = 0;
    }

    return ret;
}

int id_mgr_add_group_openid(ID_MGR_HANDLE handle, char *id, char * openid)
{
    int ret    = -1;
    int result = 0;
    char * errmsg = NULL;
    char  sq_cmd[256];

    /* 如果设备不存在则返回 */
    if (0 == device_is_exist(handle, id))
    {
        debug_error("id %s un-exist \n", id);
        return -1;
    }
    else
    {
        // insert
        memset(sq_cmd, 0, sizeof(sq_cmd));

        sprintf(sq_cmd, "update id_table set gopenid='%s' where dev_uuid = '%s'", openid, id);

        pthread_mutex_lock(&handle->mutex);
        result = sqlite3_exec(handle->id_db, sq_cmd, 0, 0, &errmsg);
        pthread_mutex_unlock(&handle->mutex);

        if (SQLITE_OK == result)
        {
            debug_print("insert dev: %s openid:%s success \n", id, openid);
            ret = 0;
        }
        else
        {
            debug_error("insert dev: %s openid:%s failed \n", id, openid);
        }
    }

    return ret;
}


int id_mgr_del_group_openid(ID_MGR_HANDLE handle, char *id)
{
    int ret    = -1;
    int result = 0;
    char * errmsg = NULL;
    char  sq_cmd[256];

    /* 如果设备不存在则返回 */
    if (0 == device_is_exist(handle, id))
    {
        debug_error("id %s un-exist \n", id);
        return -1;
    }
    else
    {
        // insert
        memset(sq_cmd, 0, sizeof(sq_cmd));

        sprintf(sq_cmd, "update id_table set gopenid='%s' where dev_uuid = '%s'", "null", id);

        pthread_mutex_lock(&handle->mutex);
        result = sqlite3_exec(handle->id_db, sq_cmd, 0, 0, &errmsg);
        pthread_mutex_unlock(&handle->mutex);

        if (SQLITE_OK == result)
        {
            debug_print("insert dev: %s openid:%s success \n", id, "null");
            ret = 0;
        }
        else
        {
            debug_error("insert dev: %s openid:%s failed \n", id, "null");
        }
    }

    return ret;
}

int id_mgr_get_uuid_by_group_openid(ID_MGR_HANDLE handle, char * openid, char * buf, int buf_len)
{
    int ret    = -1;
    int result = 0;
    char * errmsg = NULL;
    char **dbResult;
    int nRow, nColumn;
    char sq_cmd[256];

    // insert
    memset(sq_cmd, 0, sizeof(sq_cmd));
    sprintf(sq_cmd, "select dev_uuid from id_table where gopenid = '%s'", openid);
    result = sqlite3_get_table(handle->id_db, sq_cmd, &dbResult, &nRow, &nColumn, &errmsg );
    if (SQLITE_OK == result)
    {
        // result store in dbResult[]
        if ((nRow > 0) && (nColumn > 0))
        {
            if ((NULL != dbResult[nRow]) && (strlen(dbResult[nRow]) > 0))
            {
                strncpy(buf, dbResult[nRow], buf_len);
                debug_info("get dev_uuid %s \n", buf);
                ret = 0;
            }
            else
            {
                debug_error("get pad error %d %d %s \n", nRow, nColumn, dbResult[nRow] );
            }
        }
    }

    return ret;
}

int id_mgr_id_is_exist(ID_MGR_HANDLE handle, char * id)
{
    return device_is_exist(handle, id);
}

int id_mgr_group_openid_is_exist(ID_MGR_HANDLE handle, char * openid)
{
    return group_openid_is_exist(handle, openid);
}

// delete db in client/device.
int id_mgr_del_device(ID_MGR_HANDLE handle, char *id)
{
    // del id section in client dev_list section.
    if (1 == device_is_exist(handle, id))
    {
        // delete device in device db.
        if (0 == device_del(handle, id))
        {
            debug_print("delete %s success \n", id);
        }
    }
    else
    {
        debug_print("no id  %s ?\n", id);
    }

    return 0;
}

void id_mgr_destroy(ID_MGR_HANDLE handle)
{
    if (NULL == handle)
    {
        return;
    }

    pthread_mutex_destroy(&handle->mutex);
    sqlite3_close(handle->id_db);
    free(handle);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// static function
/////////////////////////////////////////////////////////////////////////////////////////////
static ID_MGR_OBJECT* instance(void)
{
    static ID_MGR_OBJECT* handle = NULL;
    if (NULL == handle)
    {
        handle = (ID_MGR_OBJECT *)calloc(1, sizeof(ID_MGR_OBJECT));
    }

    return handle;
}

static int device_is_exist(ID_MGR_HANDLE handle, char * id)
{
    int ret = 0;
    int result = 0;
    char* errmsg = NULL;

    char sq_cmd[128];
    char **dbResult;
    int nRow, nColumn;

    memset(sq_cmd, 0, sizeof(sq_cmd));
    sprintf(sq_cmd, "select id from id_table where dev_uuid = '%s'", id);
    result = sqlite3_get_table(handle->id_db, sq_cmd, &dbResult, &nRow, &nColumn, &errmsg);
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

static int group_openid_is_exist(ID_MGR_HANDLE handle, char * openid)
{
    int ret = 0;
    int result = 0;
    char* errmsg = NULL;

    char sq_cmd[128];
    char **dbResult;
    int nRow, nColumn;

    memset(sq_cmd, 0, sizeof(sq_cmd));
    sprintf(sq_cmd, "select id from id_table where gopenid = '%s'", openid);
    result = sqlite3_get_table(handle->id_db, sq_cmd, &dbResult, &nRow, &nColumn, &errmsg);
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

static int device_del(ID_MGR_HANDLE handle, char * id)
{
    char sq_cmd[128];
    int result = 0;
    sqlite3_stmt *stmt = NULL;
    const char * errorCode = NULL;

    // delete the first item here.
    sqlite3_exec(handle->id_db, "begin", 0, 0, 0);

    memset(sq_cmd, 0, sizeof(sq_cmd));
    sprintf(sq_cmd, "delete from id_table  where dev_uuid = '%s'", id);

    pthread_mutex_lock(&handle->mutex);
    result = sqlite3_prepare(handle->id_db, sq_cmd, -1, &stmt, &errorCode);
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
    sqlite3_exec(handle->id_db, "commit", 0, 0, 0);

    return 0;
}

int id_mgr_unit_test(void)
{
    int ret = -1;
    ID_MGR_HANDLE handle = id_mgr_create();
    if (NULL == handle)
    {
        return -1;
    }
    char * id = "0011220398789983";
    ret = id_mgr_add_device(handle, id);
    if (0 != ret)
    {
        debug_error("id_mgr_add_device failed \n");
        return -1;
    }

    debug_info("id is exist: %d \n", id_mgr_id_is_exist(handle, id));

    ret = id_mgr_add_device(handle, id);
    if (0 != ret)
    {
        debug_error("id_mgr_add_device failed \n");
        return -1;
    }

    id_mgr_destroy(handle);

    return 0;
}
