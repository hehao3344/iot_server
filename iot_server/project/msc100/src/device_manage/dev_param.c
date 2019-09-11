#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "hash_table.h"
#include "db/id_mgr.h"
#include "common_tbl.h"
#include "dev_param.h"

typedef struct _GW_DEVICE
{
    int      sock_fd;
    struct   list_head list;
    char     cc_id[MAX_ID_LEN];             /* 中控的ID号 */
    SUB_DEV_NODE sub_dev;

    unsigned int hash_value;
    unsigned int next_sec;
} GW_DEVICE;

typedef struct _SOCK_FD_NODE
{
    struct   list_head list;
    int      sock_fd;
    unsigned int next_sec;
} SOCK_FD_NODE;

typedef struct _DEV_PARAM_OBJECT
{
    struct  list_head head;         // GW_DEVICE head;
    struct  list_head sock_fd_head; // accept进来但是还未注册的sock_fd

    sock_exit_callback cb;
    void * arg;

    struct  hashtable * hcc_id;

    pthread_mutex_t   mutex;
    ID_MGR_HANDLE     hid_mgr;
} DEV_PARAM_OBJECT;

static GW_DEVICE *get_device_by_id(DEV_PARAM_HANDLE handle, char *id);

DEV_PARAM_HANDLE dev_param_create(int max_dev_count)
{
    DEV_PARAM_OBJECT *handle = (DEV_PARAM_OBJECT *)calloc(1, sizeof(DEV_PARAM_OBJECT));
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
    //id_mgr_add_device(handle->hid_mgr, "1084f3eb83a7aa");

    handle->hcc_id = create_hashtable(max_dev_count, common_tbl_to_hash, common_tbl_keys_equal_func);
    INIT_LIST_HEAD(&handle->head);
    INIT_LIST_HEAD(&handle->sock_fd_head);
    pthread_mutex_init(&handle->mutex, NULL);

    return handle;

create_failed:
    dev_param_destroy(handle);
    return NULL;
}

void dev_param_set_sock_exit_cb(DEV_PARAM_HANDLE handle, sock_exit_callback cb, void *arg)
{
    if (NULL != handle)
    {
        handle->cb  = cb;
        handle->arg = arg;
    }
}

int dev_param_sock_fd_is_exist(DEV_PARAM_HANDLE handle, int sock_fd)
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

void dev_param_get_proxy_server_addr(DEV_PARAM_HANDLE handle, char *ip, int len)
{
    strncpy(ip, (char *)PROXY_SERVER_IP, len);
}

int dev_param_add_connect_sock(DEV_PARAM_HANDLE handle, int sock_fd)
{
    if (NULL == handle)
    {
        debug_error("invalid param \n");
        return -1;
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

int dev_param_register(DEV_PARAM_HANDLE handle, char *cc_id, int hash_value, int sock_fd)
{
    int should_rebuild = 0;
    if ((strlen(cc_id) > MAX_ID_LEN) || (0 == strlen(cc_id)))
    {
        debug_print("id length %d invalid \n", (int)strlen(cc_id));
        return -1;
    }

    if (0 == id_mgr_id_is_exist(handle->hid_mgr, cc_id))
    {
        debug_print("invalid cc_id [%s][register] \n", cc_id);
        return -1;
    }

    GW_DEVICE *gw_dev = get_device_by_id(handle, cc_id);

    if ((NULL == gw_dev) || ((NULL != gw_dev) && (gw_dev->hash_value != hash_value)))
    {
        should_rebuild = 1;
    }
    if ((NULL != gw_dev) && (gw_dev->hash_value != hash_value))
    {
        debug_print("re-register id %s \n", cc_id);
        pthread_mutex_lock(&handle->mutex);
        list_del(&gw_dev->list);
        hashtable_remove(handle->hcc_id, gw_dev->cc_id);
        free(gw_dev);
        pthread_mutex_unlock(&handle->mutex);
    }

    if (1 == should_rebuild)
    {
        GW_DEVICE* gw_dev_tmp = NULL;

	    gw_dev_tmp = (GW_DEVICE *)calloc(1, sizeof(GW_DEVICE));
        if (NULL == gw_dev_tmp)
        {
            debug_print("not enough memory \n");
            return -1;
        }
        strncpy(gw_dev_tmp->cc_id, cc_id, (int)sizeof(gw_dev_tmp->cc_id));

        gw_dev_tmp->sock_fd    = sock_fd;
        gw_dev_tmp->hash_value = hash_value;
        gw_dev_tmp->next_sec   = get_real_time_sec() + TCP_TIMEOUT;

        pthread_mutex_lock(&handle->mutex);

        /* 将sock_fd从sock_fd_head中删除 */
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
                    debug_info("free sock_fd %d \n", sock_fd);
                    list_del(&sock_node->list);
                    free(sock_node);
                }
            }
        }

        if (!hashtable_insert(handle->hcc_id, gw_dev_tmp->cc_id, gw_dev_tmp))
        {
            debug_print(" hashtable_insert failed \n");
	        free(gw_dev_tmp);
	        pthread_mutex_unlock(&handle->mutex);
	        return -1;
        }
        list_add_tail(&gw_dev_tmp->list, &handle->head);
        pthread_mutex_unlock(&handle->mutex);
        debug_info("add cc_id %s:%d:%d successful \n", cc_id, hash_value, sock_fd);
    }

    if (0 == dev_param_heart_beat(handle, cc_id, sock_fd))
    {
        debug_info("cc_id %s sock_fd %d heart beat \n", cc_id, sock_fd);
    }

    return 0;
}

int dev_param_heart_beat(DEV_PARAM_HANDLE handle, char *cc_id, int sock_fd)
{
    if ((strlen(cc_id) > MAX_ID_LEN) || (0 == strlen(cc_id)))
    {
        debug_print("id length %d invalid \n", (int)strlen(cc_id));
        return -1;
    }

    if (0 == id_mgr_id_is_exist(handle->hid_mgr, cc_id))
    {
        debug_print("invalid cc_id [%s][register] \n", cc_id);
        return -1;
    }

    GW_DEVICE *gw_dev = get_device_by_id(handle, cc_id);
    if (NULL != gw_dev)
    {
        pthread_mutex_lock(&handle->mutex);
        gw_dev->next_sec   = get_real_time_sec() + TCP_TIMEOUT; // 120 secs.
        pthread_mutex_unlock(&handle->mutex);

        debug_info("cc %s heart_beat \n", cc_id);
    }
    else
    {
        debug_info("cc_id:%s unexist \n", cc_id);
        return -1;
    }

    if (0 == dev_param_sock_fd_is_exist(handle, sock_fd))
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

int dev_param_update(DEV_PARAM_HANDLE handle, char *cc_id, SUB_DEV_NODE * sub_dev)
{
    if ((strlen(cc_id) > MAX_ID_LEN) || (0 == strlen(cc_id)) || (NULL == sub_dev))
    {
        debug_print("id length %d invalid \n", (int)strlen(cc_id));
        return -1;
    }

    if (0 == id_mgr_id_is_exist(handle->hid_mgr, cc_id))
    {
        debug_print("invalid cc_id [%s][register] \n", cc_id);
        return -1;
    }

    GW_DEVICE *gw_dev = get_device_by_id(handle, cc_id);
    if (NULL != gw_dev)
    {
        int i;
        pthread_mutex_lock(&handle->mutex);
        for(i=0; i<MAX_ID_ARRAY; i++)
        {
            strncpy(gw_dev->sub_dev.id[i], sub_dev->id[i], (int)sizeof(gw_dev->sub_dev.id[i]));
            gw_dev->sub_dev.on_off[i]  = sub_dev->on_off[i];
            gw_dev->sub_dev.on_line[i] = sub_dev->on_line[i];
        }
        gw_dev->next_sec   = get_real_time_sec() + TCP_TIMEOUT; // 120 secs.
        pthread_mutex_unlock(&handle->mutex);

        debug_info("cc %s updated \n", cc_id);
    }
    else
    {
        debug_info("cc_id:%s unexist \n", cc_id);
        return -1;
    }

    return 0;
}

int dev_param_id_is_valid(DEV_PARAM_HANDLE handle, char *cc_id)
{
    int ret = -1;

    if (!id_mgr_id_is_exist(handle->hid_mgr, cc_id))
    {
        debug_print("invalid id [%s][client visit] \n", cc_id);
        return FALSE;
    }

    GW_DEVICE *gw_dev = get_device_by_id(handle, cc_id);
    if (NULL != gw_dev)
    {
        ret = 0;
    }

    return ret;
}

int dev_param_id_is_exist(DEV_PARAM_HANDLE handle, char *cc_id)
{
    return id_mgr_id_is_exist(handle->hid_mgr, cc_id);
}

void dev_param_keep_alive(DEV_PARAM_HANDLE handle, char *cc_id)
{
    GW_DEVICE * gw_dev = get_device_by_id(handle, cc_id);
    if (NULL != gw_dev)
    {
        gw_dev->next_sec = get_real_time_sec() + TCP_TIMEOUT;
    }
}

void dev_param_flush(DEV_PARAM_HANDLE handle)
{
    GW_DEVICE* gw_dev_list = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    list_for_each_safe(pos, n, &handle->head)
    {
        gw_dev_list = list_entry(pos, GW_DEVICE, list);
        if (NULL != gw_dev_list)
        {
            if (get_real_time_sec() >= (int)gw_dev_list->next_sec)
            {
                debug_info("cc  %s exit \n", gw_dev_list->cc_id);
                pthread_mutex_lock(&handle->mutex);

                tcp_close(gw_dev_list->sock_fd);

                if (NULL != handle->cb)
                {
                    handle->cb(handle->arg, gw_dev_list->sock_fd);
                }

                // 接收数据出错或者客户端已经关闭
                gw_dev_list->sock_fd = -1;

                list_del(&gw_dev_list->list);
                hashtable_remove(handle->hcc_id, gw_dev_list->cc_id);
                free(gw_dev_list);
                pthread_mutex_unlock(&handle->mutex);
            }
        }
    }
}

void dev_param_sock_fd_flush(DEV_PARAM_HANDLE handle)
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

int dev_param_get_sub_dev_node(DEV_PARAM_HANDLE handle, char *cc_id, SUB_DEV_NODE * sub_dev_node)
{
    int ret = -1;
    struct list_head   *pos = NULL;
    struct list_head   *n   = NULL;
    GW_DEVICE* gw_dev_list = NULL;
    GW_DEVICE* gw_dev_tmp = NULL;
	gw_dev_tmp = (GW_DEVICE * )hashtable_search(handle->hcc_id, cc_id);
	if ((NULL == gw_dev_tmp) || (NULL == sub_dev_node))
	{
	    debug_error("can't find cc_id %s \n", cc_id);
		return -1;
	}

    list_for_each_safe(pos, n, &handle->head)
    {
        gw_dev_list = list_entry(pos, GW_DEVICE, list);
        if ((NULL != gw_dev_list) && (0 == strcmp(cc_id, gw_dev_list->cc_id)))
        {
            pthread_mutex_lock(&handle->mutex);
            memcpy(sub_dev_node, &gw_dev_list->sub_dev, sizeof(SUB_DEV_NODE));
            pthread_mutex_unlock(&handle->mutex);
            ret = 0;
            break;
        }
    }

	return ret;
}

int dev_param_remove(DEV_PARAM_HANDLE handle, char *cc_id)
{
    int ret   = -1;
    struct list_head   *pos = NULL;
    struct list_head   *n   = NULL;
    GW_DEVICE* gw_dev_list = NULL;
    GW_DEVICE* gw_dev_tmp = NULL;
	gw_dev_tmp = (GW_DEVICE * )hashtable_search(handle->hcc_id, cc_id);
	if (NULL == gw_dev_tmp)
	{
		return -1;
	}

    list_for_each_safe(pos, n, &handle->head)
    {
        gw_dev_list = list_entry(pos, GW_DEVICE, list);
        if (NULL != gw_dev_list)
        {
            pthread_mutex_lock(&handle->mutex);
            list_del(&gw_dev_list->list);
            free(gw_dev_list);
            pthread_mutex_unlock(&handle->mutex);
            break;
        }
    }

	if (0 == hashtable_remove(handle->hcc_id, cc_id))
	{
		ret = 0;
	}

	return ret;
}

int dev_param_get_count(DEV_PARAM_HANDLE handle)
{
	return (int)hashtable_count(handle->hcc_id);
}

int dev_param_get_sock_fd(DEV_PARAM_HANDLE handle, char *cc_id, int *sock_fd)
{
    int ret = -1;
    GW_DEVICE *client = (GW_DEVICE *)hashtable_search(handle->hcc_id, cc_id);
    if (NULL != client)
    {
        *sock_fd = client->sock_fd;
        ret = 0;
    }

	return ret;
}

void dev_param_dump(DEV_PARAM_HANDLE handle)
{
    int   count;
    char  buffer[64];
    char  commom_time[32];
    char  ymd_buf[16];
    char  filename[32];

    count = (int)hashtable_count(handle->hcc_id);

    debug_print("device info: \n");
    debug_print("total [%d] device registered \n", count);

    memset(filename, 0, sizeof(filename));
    memset(ymd_buf, 0, sizeof(ymd_buf));
    // get_time_ymd(ymd_buf);
    sprintf(filename, "%s.log", ymd_buf);

    FILE *fp = fopen(filename, "a+");
    memset(buffer, 0, sizeof(buffer));
    memset(commom_time, 0, sizeof(commom_time));
    get_common_time(commom_time);
    sprintf(buffer, "%s - count:[%d]\r\n", commom_time, count );

    if (NULL != fp)
    {
        fwrite(buffer, 1, strlen(buffer), fp);
        fclose(fp);
    }
}

void dev_param_destroy(DEV_PARAM_HANDLE handle)
{
    GW_DEVICE* gw_dev_list = NULL;

    SOCK_FD_NODE * sock_node = NULL;

    struct list_head *pos = NULL;
    struct list_head *n   = NULL;
    if (NULL == handle)
    {
        return;
    }
    list_for_each_safe(pos, n, &handle->head)
    {
        gw_dev_list = list_entry(pos, GW_DEVICE, list);
        if (NULL != gw_dev_list)
        {
            list_del(&gw_dev_list->list);
            free(gw_dev_list);
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
    hashtable_destroy(handle->hcc_id, 0);
    pthread_mutex_destroy(&handle->mutex);
    free(handle);
}

/*******************************************************************************/
/*			Private Function Definition										   */
/*******************************************************************************/
static GW_DEVICE *get_device_by_id(DEV_PARAM_HANDLE handle, char *cc_id)
{
	return (GW_DEVICE*)hashtable_search(handle->hcc_id, cc_id);
}
