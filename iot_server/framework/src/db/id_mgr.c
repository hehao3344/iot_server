#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "sqlite3.h"
#include "id_mgr.h"

#define MAX_ID_NUMBER                   (20000)

#define CREATE_ID_STRING                "create table id_table(id integer primary key autoincrement, \
dev_uuid char(16), gopenid char(32), openid1 char(32), \
openid2 char(32), openid3 char(32), \
dev_name char(32), product_key char(32), dev_secret char(64), \
last_online_time char(24), last_offline_time char(24), online int)"
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

int id_mgr_add_device(ID_MGR_HANDLE handle, char *id, char * product_key, char *dev_secret)
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
        snprintf(sq_cmd, sizeof(sq_cmd), "insert into id_table(dev_uuid, product_key, dev_secret, online) values ('%s', '%s', '%s', '%d')", id, product_key, dev_secret, 0);

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

// 0 成功 1 该设备已经被绑定 2 设备不存在 -1 失败
int id_mgr_add_group_openid(ID_MGR_HANDLE handle, char *id, char * openid)
{
    int ret    = -1;
    int result = 0;
    char * errmsg = NULL;
    int nRow, nColumn;
    char **dbResult;
    char  sq_cmd[256];

    /* 如果设备不存在则返回 */
    if (0 == device_is_exist(handle, id))
    {
        debug_error("id %s un-exist \n", id);
        return 2;
    }
    else
    {
        int should_insert = 0;
        // insert
        memset(sq_cmd, 0, sizeof(sq_cmd));

        sprintf(sq_cmd, "select gopenid from id_table where dev_uuid = '%s'", id);
        result = sqlite3_get_table(handle->id_db, sq_cmd, &dbResult, &nRow, &nColumn, &errmsg );
        if (SQLITE_OK == result)
        {
            if ((nRow > 0) && (nColumn > 0))
            {
                if ((NULL != dbResult[nRow]) && (strlen(dbResult[nRow]) > 0))
                {
                    debug_print("dev: %s openid %s exist already \n", id, openid);
                    ret = 1;
                }
                else
                {
                    should_insert = 1;
                }
            }
            else
            {
                should_insert = 1;
            }
        }
        else
        {
            should_insert = 1;
        }

        if (1 == should_insert)
        {
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
    }

    return ret;
}

int id_mgr_update_dev_name(ID_MGR_HANDLE handle, char *id, char * dev_name)
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

        sprintf(sq_cmd, "update id_table set dev_name='%s' where dev_uuid = '%s'", dev_name, id);

        pthread_mutex_lock(&handle->mutex);
        result = sqlite3_exec(handle->id_db, sq_cmd, 0, 0, &errmsg);
        pthread_mutex_unlock(&handle->mutex);

        if (SQLITE_OK == result)
        {
            debug_print("insert dev: %s dev_name:%s success \n", id, dev_name);
            ret = 0;
        }
        else
        {
            debug_error("insert dev: %s dev_name:%s failed \n", id, dev_name);
        }
    }

    return ret;
}

int id_mgr_update_product_key(ID_MGR_HANDLE handle, char *id, char * product_key)
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

        sprintf(sq_cmd, "update id_table set product_key='%s' where dev_uuid = '%s'", product_key, id);

        pthread_mutex_lock(&handle->mutex);
        result = sqlite3_exec(handle->id_db, sq_cmd, 0, 0, &errmsg);
        pthread_mutex_unlock(&handle->mutex);

        if (SQLITE_OK == result)
        {
            debug_print("insert dev: %s product_key:%s success \n", id, product_key);
            ret = 0;
        }
        else
        {
            debug_error("insert dev: %s product_key:%s failed \n", id, product_key);
        }
    }

    return ret;
}

int id_mgr_update_dev_secret(ID_MGR_HANDLE handle, char *id, char * dev_secret)
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

        sprintf(sq_cmd, "update id_table set dev_secret='%s' where dev_uuid = '%s'", dev_secret, id);

        pthread_mutex_lock(&handle->mutex);
        result = sqlite3_exec(handle->id_db, sq_cmd, 0, 0, &errmsg);
        pthread_mutex_unlock(&handle->mutex);

        if (SQLITE_OK == result)
        {
            debug_print("insert dev: %s dev_secret:%s success \n", id, dev_secret);
            ret = 0;
        }
        else
        {
            debug_error("insert dev: %s dev_secret:%s failed \n", id, dev_secret);
        }
    }

    return ret;
}

int id_mgr_update_online_time(ID_MGR_HANDLE handle, char *id, char * last_online_time)
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
        sprintf(sq_cmd, "update id_table set last_online_time='%s', online='%d' where dev_uuid = '%s'", last_online_time, 1, id);

        pthread_mutex_lock(&handle->mutex);
        result = sqlite3_exec(handle->id_db, sq_cmd, 0, 0, &errmsg);
        pthread_mutex_unlock(&handle->mutex);

        if (SQLITE_OK == result)
        {
            debug_print("insert dev: %s last_online_time:%s success \n", id, last_online_time);
            ret = 0;
        }
        else
        {
            debug_error("insert dev: %s last_online_time:%s failed \n", id, last_online_time);
        }
    }

    return ret;
}

int id_mgr_update_offline_time(ID_MGR_HANDLE handle, char *id, char * last_offline_time)
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

        // 开始上线 开始时间=结束时间
        sprintf(sq_cmd, "update id_table set last_offline_time='%s', online='%d' where dev_uuid = '%s'", last_offline_time, 0, id);

        pthread_mutex_lock(&handle->mutex);
        result = sqlite3_exec(handle->id_db, sq_cmd, 0, 0, &errmsg);
        pthread_mutex_unlock(&handle->mutex);

        if (SQLITE_OK == result)
        {
            debug_print("insert dev: %s last_offline_time:%s success \n", id, last_offline_time);
            ret = 0;
        }
        else
        {
            debug_error("insert dev: %s last_offline_time:%s failed \n", id, last_offline_time);
        }
    }

    return ret;
}

/* 0成功 1无该设备 2 设备未被绑定 3解绑失败*/
int id_mgr_del_group_openid(ID_MGR_HANDLE handle, char *id, char * gopenid)
{
    int ret    = -1;
    int result = 0;
    char * errmsg = NULL;
    char  sq_cmd[256];

    /* 如果设备不存在则返回 */
    if (0 == device_is_exist(handle, id))
    {
        debug_error("id %s un-exist \n", id);
        return 1;
    }
    else if(0 == id_mgr_group_openid_is_exist(handle, gopenid))
    {
        debug_error("openid %s un-exist \n", gopenid);
        return 1;
    }
    else
    {
        // insert
        memset(sq_cmd, 0, sizeof(sq_cmd));

        sprintf(sq_cmd, "update id_table set gopenid='%s' where dev_uuid = '%s'", "", id);

        pthread_mutex_lock(&handle->mutex);
        result = sqlite3_exec(handle->id_db, sq_cmd, 0, 0, &errmsg);
        pthread_mutex_unlock(&handle->mutex);

        if (SQLITE_OK == result)
        {
            debug_print("delete dev: %s openid:%s success \n", id, "");
            ret = 0;
        }
        else
        {
            debug_error("delete dev: %s openid:%s failed \n", id, "");
            ret = 2;
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

int id_mgr_get_product_key_by_group_openid(ID_MGR_HANDLE handle, char * openid, char * buf, int buf_len)
{
    int ret    = -1;
    int result = 0;
    char * errmsg = NULL;
    char **dbResult;
    int nRow, nColumn;
    char sq_cmd[256];

    // insert
    memset(sq_cmd, 0, sizeof(sq_cmd));
    sprintf(sq_cmd, "select product_key from id_table where gopenid = '%s'", openid);
    result = sqlite3_get_table(handle->id_db, sq_cmd, &dbResult, &nRow, &nColumn, &errmsg );
    if (SQLITE_OK == result)
    {
        // result store in dbResult[]
        if ((nRow > 0) && (nColumn > 0))
        {
            if ((NULL != dbResult[nRow]) && (strlen(dbResult[nRow]) > 0))
            {
                strncpy(buf, dbResult[nRow], buf_len);
                debug_info("get product_key %s \n", buf);
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

int id_mgr_get_dev_secret_by_group_openid(ID_MGR_HANDLE handle, char * openid, char * buf, int buf_len)
{
    int ret    = -1;
    int result = 0;
    char * errmsg = NULL;
    char **dbResult;
    int nRow, nColumn;
    char sq_cmd[256];

    // insert
    memset(sq_cmd, 0, sizeof(sq_cmd));
    sprintf(sq_cmd, "select dev_secret from id_table where gopenid = '%s'", openid);
    result = sqlite3_get_table(handle->id_db, sq_cmd, &dbResult, &nRow, &nColumn, &errmsg );
    if (SQLITE_OK == result)
    {
        // result store in dbResult[]
        if ((nRow > 0) && (nColumn > 0))
        {
            if ((NULL != dbResult[nRow]) && (strlen(dbResult[nRow]) > 0))
            {
                strncpy(buf, dbResult[nRow], buf_len);
                debug_info("get dev_secret %s \n", buf);
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

int id_mgr_get_uuid(ID_MGR_HANDLE handle, got_callback fn_cb, void * arg)
{
    int ret    = -1;
    int result = 0;
    // int index  = 0;
    char * errmsg = NULL;
    char **dbResult;
    int nRow, nColumn;
    char sq_cmd[256];
    int  id = 1;
    char dev_uuid[16] = {0};
    char openid[32] = {0};
    char dev_name[32] = {0};
    char dev_secret[64] = {0};
    char product_key[32] = {0};

    char last_online_time[24] = {0};
    char last_offline_time[24] = {0};
    int  online = 0;
    memset(sq_cmd, 0, sizeof(sq_cmd));
    //sprintf(sq_cmd, "select dev_uuid from id_table");
    sprintf(sq_cmd, "select * from id_table");
    result = sqlite3_get_table(handle->id_db, sq_cmd, &dbResult, &nRow, &nColumn, &errmsg );
    if (SQLITE_OK == result)
    {
        // result store in dbResult[]
        if ((nRow > 0) && (nColumn > 0))
        {
            int i, j;
            int cur_index = nColumn;
            // dbResult 的字段值是连续的，从第0索引到第 nColumn - 1索引都是字段名称，
            // 从第 nColumn 索引开始，后面都是字段值，它把一个二维的表（传统的行列表示法）用一个扁平的形式来表示
            for(i=0; i<nRow; i++)
            {
                memset(dev_uuid, 0, sizeof(dev_uuid));
                memset(dev_name, 0, sizeof(dev_name));
                memset(last_online_time, 0, sizeof(last_online_time));
                memset(last_offline_time, 0, sizeof(last_offline_time));

                for(j=0; j<nColumn; j++)
                {
                    //debug_info("get j=%d %s %s \n", j, dbResult[j], dbResult[cur_index]);
                    if ((NULL == dbResult[cur_index]) || (0 == strlen(dbResult[cur_index])))
                    {
                        cur_index++;
                        continue;
                    }
                    if (0 == strcmp(dbResult[j], "id"))
                    {
                        id = atoi(dbResult[cur_index]);
                    }
                    else if (0 == strcmp(dbResult[j], "dev_uuid"))
                    {
                        strncpy(dev_uuid, dbResult[cur_index], sizeof(dev_uuid));
                    }
                    else if (0 == strcmp(dbResult[j], "gopenid"))
                    {
                        strncpy(openid, dbResult[cur_index], sizeof(openid));
                    }
                    else if (0 == strcmp(dbResult[j], "dev_name"))
                    {
                        strncpy(dev_name, dbResult[cur_index], sizeof(dev_name));
                    }
                    else if (0 == strcmp(dbResult[j], "product_key"))
                    {
                        strncpy(product_key, dbResult[cur_index], sizeof(product_key));
                    }
                    else if (0 == strcmp(dbResult[j], "dev_secret"))
                    {
                        strncpy(dev_secret, dbResult[cur_index], sizeof(dev_secret));
                    }
                    else if (0 == strcmp(dbResult[j], "last_online_time"))
                    {
                        strncpy(last_online_time, dbResult[cur_index], sizeof(last_online_time));
                    }
                    else if (0 == strcmp(dbResult[j], "last_offline_time"))
                    {
                        strncpy(last_offline_time, dbResult[cur_index], sizeof(last_offline_time));
                    }
                    else if (0 == strcmp(dbResult[j], "online"))
                    {
                        online = atoi(dbResult[cur_index]);
                    }

                    cur_index++;
                }
                // debug_info("get nRow=%d nColumn=%d %d index = %d %s %s\n", nRow, nColumn, index, cur_index, dbResult[j], dbResult [cur_index]);
                if (NULL != fn_cb)
                {
                    fn_cb(arg, id, dev_uuid, openid, dev_name, product_key, dev_secret, last_online_time, last_offline_time, online);
                    ret = 0;
                }
#if 0
                for(j=0; j<nColumn; j++)
                {

                    // debug_info("get nRow=%d nColumn=%d %d index = %d %s %s\n", nRow, nColumn, index, cur_index, dbResult[j], dbResult [cur_index]);
                    if (NULL != fn_cb)
                    {
                        fn_cb(arg, index, dbResult[cur_index], "dev_name", "dev_time");

                        debug_info("get %s %s \n", dbResult[j], dbResult[cur_index]);
                        index++;
                        ret = 0;
                    }
                    cur_index++;
                }
#endif
            }
        }
    }

    return ret;
}

int id_mgr_get_uuid_by_index(ID_MGR_HANDLE handle, int index, char * buf, int buf_len)
{
    int ret    = -1;
    int result = 0;
    char * errmsg = NULL;
    char **dbResult;
    int nRow, nColumn;
    char sq_cmd[256];

    memset(sq_cmd, 0, sizeof(sq_cmd));
    sprintf(sq_cmd, "select dev_uuid from id_table");
    result = sqlite3_get_table(handle->id_db, sq_cmd, &dbResult, &nRow, &nColumn, &errmsg );
    if (SQLITE_OK == result)
    {
        // result store in dbResult[]
        if ((nRow > 0) && (nColumn > 0))
        {
            int i, j;
            int cur_index = nColumn;
            // dbResult 的字段值是连续的，从第0索引到第 nColumn - 1索引都是字段名称，
            // 从第 nColumn 索引开始，后面都是字段值，它把一个二维的表（传统的行列表示法）用一个扁平的形式来表示
            for(i=0; i<nRow; i++)
            {
                if (index == i)
                {
                    for(j=0; j<nColumn; j++)
                    {
                        // debug_info("get nRow=%d nColumn=%d %d index = %d %s %s\n", nRow, nColumn, index, cur_index, dbResult[j], dbResult [cur_index]);
                        strncpy(buf, dbResult[cur_index], buf_len-1);
                    }
                    ret = 0;
                    break;
                }
                cur_index++;
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
    sprintf(sq_cmd, "select gopenid from id_table where gopenid = '%s'", openid);
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
    ret = id_mgr_add_device(handle, id, "1", "2");
    if (0 != ret)
    {
        debug_error("id_mgr_add_device failed \n");
        return -1;
    }

    debug_info("id is exist: %d \n", id_mgr_id_is_exist(handle, id));

    ret = id_mgr_add_device(handle, id, "3", "4");
    if (0 != ret)
    {
        debug_error("id_mgr_add_device failed \n");
        return -1;
    }

    id_mgr_destroy(handle);

    return 0;
}
