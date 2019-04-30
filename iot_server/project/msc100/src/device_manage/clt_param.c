#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "hash_table.h"
#include "db/id_mgr.h"
#include "clt_param.h"


#define TCP_TIMEOUT     20

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
    struct  list_head sock_fd_head; // accept进来但是还未注册的sock_fd

    sock_exit_callback cb;
    void * arg;
    struct  hashtable * hclient;
    pthread_mutex_t   mutex;
    ID_MGR_HANDLE     hid_mgr;
} CLT_PARAM_OBJECT;

static long g_lCryptTable[] =
{
    0x697A5,0x6045C,0xAB4E2,0x409E4,0x71209,0x32392,0xA7292,0xB09FC,0x4B658,0xAAAD5,0x9B9CF,0xA326A,0x8DD12,0x38150,0x8E14D,0x2EB7F,
    0xE0A56,0x7E6FA,0xDFC27,0xB1301,0x8B4F7,0xA7F70,0xAA713,0x6CC0F,0x6FEDF,0x2EC87,0xC0F1C,0x45CA4,0x30DF8,0x60E99,0xBC13E,0x4E0B5,
    0x6318B,0x82679,0x26EF2,0x79C95,0x86DDC,0x99BC0,0xB7167,0x72532,0x68765,0xC7446,0xDA70D,0x9D132,0xE5038,0x2F755,0x9171F,0xCB49E,
    0x6F925,0x601D3,0x5BD8A,0x2A4F4,0x9B022,0x706C3,0x28C10,0x2B24B,0x7CD55,0xCA355,0xD95F4,0x727BC,0xB1138,0x9AD21,0xC0ACA,0xCD928,
    0x953E5,0x97A20,0x345F3,0xBDC03,0x7E157,0x96C99,0x968EF,0x92AA9,0xC2276,0xA695D,0x6743B,0x2723B,0x58980,0x66E08,0x51D1B,0xB97D2,
    0x6CAEE,0xCC80F,0x3BA6C,0xB0BF5,0x9E27B,0xD122C,0x48611,0x8C326,0xD2AF8,0xBB3B7,0xDED7F,0x4B236,0xD298F,0xBE912,0xDC926,0xC873F,
    0xD0716,0x9E1D3,0x48D94,0x9BD91,0x5825D,0x55637,0xB2057,0xBCC6C,0x460DE,0xAE7FB,0x81B03,0x34D8F,0xC0528,0xC9B59,0x3D260,0x6051D,
    0x93757,0x8027F,0xB7C34,0x4A14E,0xB12B8,0xE4945,0x28203,0xA1C0F,0xAA382,0x46ABB,0x330B9,0x5A114,0xA754B,0xC68D0,0x9040E,0x6C955,
    0xBB1EF,0x51E6B,0x9FF21,0x51BCA,0x4C879,0xDFF70,0x5B5EE,0x29936,0xB9247,0x42611,0x2E353,0x26F3A,0x683A3,0xA1082,0x67333,0x74EB7,
    0x754BA,0x369D5,0x8E0BC,0xABAFD,0x6630B,0xA3A7E,0xCDBB1,0x8C2DE,0x92D32,0x2F8ED,0x7EC54,0x572F5,0x77461,0xCB3F5,0x82C64,0x35FE0,
    0x9203B,0xADA2D,0xBAEBD,0xCB6AF,0xC8C9A,0x5D897,0xCB727,0xA13B3,0xB4D6D,0xC4929,0xB8732,0xCCE5A,0xD3E69,0xD4B60,0x89941,0x79D85,
    0x39E0F,0x6945B,0xC37F8,0x77733,0x45D7D,0x25565,0xA3A4E,0xB9F9E,0x316E4,0x36734,0x6F5C3,0xA8BA6,0xC0871,0x42D05,0x40A74,0x2E7ED,
    0x67C1F,0x28BE0,0xE162B,0xA1C0F,0x2F7E5,0xD505A,0x9FCC8,0x78381,0x29394,0x53D6B,0x7091D,0xA2FB1,0xBB942,0x29906,0xC412D,0x3FCD5,
    0x9F2EB,0x8F0CC,0xE25C3,0x7E519,0x4E7D9,0x5F043,0xBBA1B,0x6710A,0x819FB,0x9A223,0x38E47,0xE28AD,0xB690B,0x42328,0x7CF7E,0xAE108,
    0xE54BA,0xBA5A1,0xA09A6,0x9CAB7,0xDB2B3,0xA98CC,0x5CEBA,0x9245D,0x5D083,0x8EA21,0xAE349,0x54940,0x8E557,0x83EFD,0xDC504,0xA6059,
    0xB85C9,0x9D162,0x7AEB6,0xBED34,0xB4963,0xE367B,0x4C891,0x9E42C,0xD4304,0x96EAA,0xD5D69,0x866B8,0x83508,0x7BAEC,0xD03FD,0xDA122
};

/************************************************************************************************************************/
/*			Internal Visiable Function Reference																		*/
/************************************************************************************************************************/
static unsigned int to_hash (void* pstring);
static int keys_equal_func (void *pkey1, void *pkey2);
static APP_DEVICE *get_device_by_id(CLT_PARAM_HANDLE handle, char *id);

/************************************************************************************************************************/
/*			Internal Visiable Constant Definition																		*/
/************************************************************************************************************************/
// if you find 256 entries too excessive for your liking
// you may desize it, but I recommend a power-of-two size
// (if you change the size, you must change the AND statement
// in the function below)
#define TYPE            (0x9C)                  /* If You Change This, You Change The Hash Output! */

/************************************************************************************************************************/
/*			Public Function Definition																					*/
/************************************************************************************************************************/
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

    handle->hclient = create_hashtable(max_clt_count, to_hash, keys_equal_func);

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

int clt_heart_beat(CLT_PARAM_HANDLE handle, char *gopenid)
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

        debug_info("app_openid %s heart_beat \n", gopenid);
    }
    else
    {
        debug_info("app_openid:%s unexist \n", gopenid);
        return -1;
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
                debug_info("gopenid  %s exit \n", app_dev_list->open_id);
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
static APP_DEVICE *get_device_by_id(CLT_PARAM_HANDLE handle, char *cc_id)
{
	return (APP_DEVICE*)hashtable_search(handle->hclient, cc_id);
}

static unsigned int to_hash(void* pstring)
{
	unsigned int nseed1, nseed2;

	char* pkey = (char*)pstring;
	char  ch;

	// LOL, coder joke: Dead Code ;)
	nseed1 = 0xDEADC0DE;
	nseed2 = 0x7FED7FED;

	while (*pkey != 0)
	{
		ch = toupper (*pkey++);

		// if you changed the size of the g_lCryptTable, you must change the & 0xFF below
		// to & whatever if it's a power of two, or % whatever, if it's not
		nseed1 = g_lCryptTable[((TYPE << 8) + ch)&0xFF] ^ (nseed1 + nseed2);
		nseed2 = ch + nseed1 + nseed2 + (nseed2 << 5) + 3;
	}

	return nseed1;
}

static int keys_equal_func(void *pkey1, void *pkey2)
{
    return 1;
}
