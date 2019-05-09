#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "hash_table.h"
#include "db/id_mgr.h"
#include "common_tbl.h"
#include "clt_param.h"

typedef struct _APP_DEVICE
{
    int      sock_fd;
    struct   list_head list;
    char     open_id[MAX_OPEN_ID_LEN];
    unsigned int hash_value;
    unsigned int next_sec;
} APP_DEVICE;

typedef struct _SOCK_FD_NODE
{
    int      sock_fd;
    struct   list_head list;
    unsigned int next_sec;
} SOCK_FD_NODE;

typedef struct _CLT_PARAM_OBJECT
{
    struct  list_head head;         // GW_DEVICE head;
    struct  list_head sock_fd_head; // accept进来但是还未发送心跳的sock_fd

    sock_exit_callback cb;
    void * arg;
    struct  hashtable * hclient;
    pthread_mutex_t   mutex;
    ID_MGR_HANDLE     hid_mgr;
} CLT_PARAM_OBJECT;

static APP_DEVICE *get_device_by_id(CLT_PARAM_HANDLE handle, char *id);
static int add_device(CLT_PARAM_HANDLE handle, char *gopenid, int sock_fd);

CLT_PARAM_HANDLE clt_param_create(int max_clt_count)
{
    CLT_PARAM_OBJECT *handle = (CLT_PARAM_OBJECT *)calloc(1, sizeof(CLT_PARAM_OBJECT));
	if (NULL == handle)
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    handle->hid_mgr = id_mgr_create();
    if (NULL == handle->hid_mgr)
    {
        debug_error("id_mgr_create failed \n");
        goto create_failed;
    }

    //id_mgr_add_device(handle->hid_mgr, "10001122334455");
    id_mgr_add_group_openid(handle->hid_mgr, "10001122334455", "ogDt75W7bJt-DTubZvPFrQCZ8Y58");
    id_mgr_add_group_openid(handle->hid_mgr, "1084f3eb83a7aa", "ogDt75W7bJt-DTubZvPFrQCZ8Y58");

    handle->hclient = create_hashtable(max_clt_count, common_tbl_to_hash, common_tbl_keys_equal_func);

    INIT_LIST_HEAD(&handle->head);
    INIT_LIST_HEAD(&handle->sock_fd_head);
    pthread_mutex_init(&handle->mutex, NULL);

    return handle;

create_failed:
    clt_param_destroy(handle);
    return NULL;
}

void clt_param_set_sock_exit_cb(CLT_PARAM_HANDLE handle, sock_exit_callback cb, void *arg)
{
    if (NULL != handle)
    {
        handle->cb  = cb;
        handle->arg = arg;
    }
}

int clt_param_sock_fd_is_exist(CLT_PARAM_HANDLE handle, int sock_fd)
{
    int ret = 0;
    SOCK_FD_NODE * sock_node = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    list_for_each_safe(pos, n, &handle->sock_fd_head)
    {
        sock_node = list_entry(pos, SOCK_FD_NODE, list);
        if (NULL != sock_node)
        {
            if (sock_fd == sock_node->sock_fd)
            {
                ret = 1;
                break;
            }
        }
    }

    return ret;
}

int clt_param_add_connect_sock(CLT_PARAM_HANDLE handle, int sock_fd)
{
    if (NULL == handle)
    {
        debug_error("invalid param \n");
        return -1;
    }

    if (1 == clt_param_sock_fd_is_exist(handle, sock_fd))
    {
        debug_info("sock_fd_is_exist already \n");
        return 0;
    }

    SOCK_FD_NODE * sock_node = NULL;

    sock_node = (SOCK_FD_NODE *)calloc(1, sizeof(SOCK_FD_NODE));
    if (NULL == sock_node)
    {
        debug_error("not enough memory \n");
        return -1;
    }
    pthread_mutex_lock(&handle->mutex);
    sock_node->sock_fd  = sock_fd;
    sock_node->next_sec = get_real_time_sec() + TCP_TIMEOUT;
    pthread_mutex_unlock(&handle->mutex);

    list_add_tail(&sock_node->list, &handle->sock_fd_head);
    debug_info("sock_fd %d connectted \n", sock_fd);

    return 0;
}

int clt_param_get_dev_uuid_by_openid(CLT_PARAM_HANDLE handle, char * openid, char * buf, int buf_len)
{
    return id_mgr_get_uuid_by_group_openid(handle->hid_mgr, openid, buf, buf_len);
}

int clt_param_heart_beat(CLT_PARAM_HANDLE handle, char *gopenid, int sock_fd)
{
    if (0 == strlen(gopenid))
    {
        debug_print("id length %d invalid \n", (int)strlen(gopenid));
        return -1;
    }

    if (0 == id_mgr_group_openid_is_exist(handle->hid_mgr, gopenid))
    {
        debug_print("invalid gopenid [%s][register] \n", gopenid);
        return -1;
    }

    APP_DEVICE * app_dev = get_device_by_id(handle, gopenid);
    if (NULL != app_dev)
    {
        pthread_mutex_lock(&handle->mutex);
        app_dev->next_sec   = get_real_time_sec() + TCP_TIMEOUT; // 120 secs.

        pthread_mutex_unlock(&handle->mutex);

        //debug_info("app_openid %s heart_beat next ==> %d\n", gopenid, app_dev->next_sec);
    }
    else
    {
        debug_info("app_openid:%s unexist \n", gopenid);
        if (0 != add_device(handle, gopenid, sock_fd))
        {
            debug_error("add gopenid %s sock %d failed \n", gopenid, sock_fd);
            return -1;
        }
    }

    if (0 == clt_param_sock_fd_is_exist(handle, sock_fd))
    {
        SOCK_FD_NODE * sock_node = NULL;
        sock_node = (SOCK_FD_NODE *)calloc(1, sizeof(SOCK_FD_NODE));
        if (NULL == sock_node)
        {
            debug_error("not enough memory \n");
            return -1;
        }
        pthread_mutex_lock(&handle->mutex);
        sock_node->sock_fd  = sock_fd;
        sock_node->next_sec = get_real_time_sec() + TCP_TIMEOUT;
        pthread_mutex_unlock(&handle->mutex);

        list_add_tail(&sock_node->list, &handle->sock_fd_head);
    }
    else
    {
        SOCK_FD_NODE * sock_node = NULL;
        struct list_head *pos = NULL;
        struct list_head *n   = NULL;

        list_for_each_safe(pos, n, &handle->sock_fd_head)
        {
            sock_node = list_entry(pos, SOCK_FD_NODE, list);
            if (NULL != sock_node)
            {
                sock_node->next_sec = get_real_time_sec() + TCP_TIMEOUT;
            }
        }
    }

    return 0;
}

int clt_param_group_openid_is_valid(CLT_PARAM_HANDLE handle, char *gopenid)
{
    int ret = 0;

    if (0 == id_mgr_group_openid_is_exist(handle->hid_mgr, gopenid))
    {
        debug_print("invalid id [%s][client visit] \n", gopenid);
        return -1;
    }

    APP_DEVICE *app_dev = get_device_by_id(handle, gopenid);
    if (NULL != app_dev)
    {
        ret = 1;
    }

    return ret;
}

void clt_param_flush(CLT_PARAM_HANDLE handle)
{
    APP_DEVICE* app_dev_list = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    list_for_each_safe(pos, n, &handle->head)
    {
        app_dev_list = list_entry(pos, APP_DEVICE, list);
        if (NULL != app_dev_list)
        {
            if (get_real_time_sec() >= (int)app_dev_list->next_sec)
            {
                debug_info("gopenid  %s exit %d %d \n", app_dev_list->open_id, get_real_time_sec(), (int)app_dev_list->next_sec);
                pthread_mutex_lock(&handle->mutex);

                tcp_close(app_dev_list->sock_fd);

                if (NULL != handle->cb)
                {
                    handle->cb(handle->arg, app_dev_list->sock_fd);
                }

                // 接收数据出错或者客户端已经关闭
                app_dev_list->sock_fd = -1;

                list_del(&app_dev_list->list);
                hashtable_remove(handle->hclient, app_dev_list->open_id);
                free(app_dev_list);
                pthread_mutex_unlock(&handle->mutex);
            }
        }
    }
}

void clt_param_sock_fd_flush(CLT_PARAM_HANDLE handle)
{
    SOCK_FD_NODE * sock_node = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    list_for_each_safe(pos, n, &handle->sock_fd_head)
    {
        sock_node = list_entry(pos, SOCK_FD_NODE, list);
        if (NULL != sock_node)
        {
            if (get_real_time_sec() >= (int)sock_node->next_sec)
            {
                list_del(&sock_node->list);
                // 回调
                if (NULL != handle->cb)
                {
                    debug_info("sock_fd:%d free ... \n", sock_node->sock_fd);
                    handle->cb(handle->arg, sock_node->sock_fd);
                }
                free(sock_node);
            }
        }
    }
}

int clt_param_remove(CLT_PARAM_HANDLE handle, char *gopenid)
{
    int ret   = -1;
    struct list_head   *pos = NULL;
    struct list_head   *n   = NULL;
    APP_DEVICE * app_dev_list = NULL;
    APP_DEVICE * app_dev_tmp = NULL;
	app_dev_tmp = (APP_DEVICE * )hashtable_search(handle->hclient, gopenid);
	if (NULL == app_dev_tmp)
	{
		return -1;
	}

    list_for_each_safe(pos, n, &handle->head)
    {
        app_dev_list = list_entry(pos, APP_DEVICE, list);
        if (NULL != app_dev_list)
        {
            pthread_mutex_lock(&handle->mutex);
            list_del(&app_dev_list->list);
            free(app_dev_list);
            pthread_mutex_unlock(&handle->mutex);
            break;
        }
    }

	if (0 == hashtable_remove(handle->hclient, gopenid))
	{
		ret = 0;
	}

	return ret;
}

int clt_param_get_count(CLT_PARAM_HANDLE handle)
{
	return (int)hashtable_count(handle->hclient);
}

int clt_param_get_sock_fd(CLT_PARAM_HANDLE handle, char *gopenid, int *sock_fd)
{
    int ret = -1;
    APP_DEVICE *client = (APP_DEVICE *)hashtable_search(handle->hclient, gopenid);
    if (NULL != client)
    {
        *sock_fd = client->sock_fd;
        ret = 0;
    }

	return ret;
}

void clt_param_destroy(CLT_PARAM_HANDLE handle)
{
    APP_DEVICE* app_dev_list = NULL;

    SOCK_FD_NODE * sock_node = NULL;

    struct list_head *pos = NULL;
    struct list_head *n   = NULL;
    if (NULL == handle)
    {
        return;
    }
    list_for_each_safe(pos, n, &handle->head)
    {
        app_dev_list = list_entry(pos, APP_DEVICE, list);
        if (NULL != app_dev_list)
        {
            list_del(&app_dev_list->list);
            free(app_dev_list);
        }
    }
    list_del(&handle->head);

    list_for_each_safe(pos, n, &handle->sock_fd_head)
    {
        sock_node = list_entry(pos, SOCK_FD_NODE, list);
        if (NULL != sock_node)
        {
            list_del(&sock_node->list);
            // 回调
            if (NULL != handle->cb)
            {
                handle->cb(handle->arg, sock_node->sock_fd);
            }
            free(sock_node);
        }
    }
    list_del(&handle->sock_fd_head);

    id_mgr_destroy(handle->hid_mgr);
    hashtable_destroy(handle->hclient, 0);
    pthread_mutex_destroy(&handle->mutex);
    free(handle);
}

/*******************************************************************************/
/*			Private Function Definition										   */
/*******************************************************************************/
static int add_device(CLT_PARAM_HANDLE handle, char *gopenid, int sock_fd)
{
    APP_DEVICE * app_dev = NULL;

    app_dev = (APP_DEVICE *)calloc(1, sizeof(APP_DEVICE));
    if (NULL == app_dev)
    {
        debug_print("not enough memory \n");
        return -1;
    }

    strncpy(app_dev->open_id, gopenid, (int)sizeof(app_dev->open_id));

    app_dev->sock_fd    = sock_fd;
    app_dev->hash_value = 0;
    app_dev->next_sec   = get_real_time_sec() + TCP_TIMEOUT;
    pthread_mutex_lock(&handle->mutex);

    if (!hashtable_insert(handle->hclient, app_dev->open_id, app_dev))
    {
        debug_print("hashtable_insert failed \n");
        free(app_dev);
        pthread_mutex_unlock(&handle->mutex);
        return -1;
    }
    list_add_tail(&app_dev->list, &handle->head);
    pthread_mutex_unlock(&handle->mutex);
    debug_info("add cc_id %s:%d:%d successful \n", gopenid, 0, sock_fd);

    return 0;
}

static APP_DEVICE *get_device_by_id(CLT_PARAM_HANDLE handle, char *cc_id)
{
	return (APP_DEVICE*)hashtable_search(handle->hclient, cc_id);
}
