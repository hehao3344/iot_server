#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "hash_table.h"
#include "db/id_mgr.h"
#include "device_manage.h"

typedef struct Device
{
    struct list_head list;

    int8   id[24];
    uint32 hash_value;
    int32  sock_fd;

    uint32 next_sec;
} Device;

typedef struct DeviceMgrObject
{
    struct  list_head head;  // Device head;
    struct  hashtable *hclient;
    MUTEX_HANDLE      hmutex;
    ID_MGR_HANDLE     hid_mgr;

} DeviceMgrObject;

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
static unsigned int to_hash (void* pString);
static int keys_equal_func ( void *pKey1, void *pKey2 );
static Device *get_device_by_id( DEVICE_MGR_HANDLE handle, int8 *id );
/************************************************************************************************************************/
/*			Internal Visiable Constant Definition																		*/
/************************************************************************************************************************/
// if you find 256 entries too excessive for your liking
// you may desize it, but I recommend a power-of-two size
// (if you change the size, you must change the AND statement
// in the function below)
#define TYPE    (0x9C)		/* If You Change This, You Change The Hash Output! */

/************************************************************************************************************************/
/*			Public Function Definition																					*/
/************************************************************************************************************************/
DEVICE_MGR_HANDLE device_mgr_create( int32 max_client_count )
{
    DeviceMgrObject *handle = ( DeviceMgrObject * )calloc( 1, sizeof( DeviceMgrObject ) );
	if ( NULL == handle )
    {
        debug_error("not enough memory \n");
        return NULL;
    }
    handle->hid_mgr = id_mgr_create();
    if ( NULL == handle->hid_mgr )
    {
        debug_error("id_mgr_create failed \n");
        return NULL;
    }

    handle->hclient = create_hashtable( max_client_count, to_hash, keys_equal_func );
    INIT_LIST_HEAD( &handle->head );

    os_mutex_open( &handle->hmutex, NULL );

    return handle;
}

boolean device_mgr_get_proxy_server_addr( DEVICE_MGR_HANDLE handle, int8 *id,  int8 *ip )
{
    boolean ret = FALSE;
    int32   position = id_mgr_get_id_position( handle->hid_mgr, id );

    if ( ( position >= 0 ) && ( position < 10000 ) )
    {
        strcpy( ip, (int8 *)"120.24.210.85" );
        //strcpy( ip, (int8 *)"192.168.1.15" );
        ret = TRUE;
    }
    else
    {
        debug_error("not enough proxy server now ! \n");
    }

    return ret;
}

boolean device_mgr_add( DEVICE_MGR_HANDLE handle, int8 *id, int32 sock_fd, uint32 hash_value )
{
    if ( 20 != strlen( id ) )
    {
        debug_print( "id invalid \n" );
        return FALSE;
    }

    if ( !id_mgr_id_is_exist( handle->hid_mgr, id ) )
    {
        debug_print( "invalid id [%s][terminal register] \n", id );
        return FALSE;
    }

    Device *clt = get_device_by_id( handle, id );
    if ( NULL == clt )
    {
        Device* clt_tmp = NULL;

	    clt_tmp = ( Device * )calloc( 1, sizeof( Device ) );
        if ( NULL == clt_tmp )
        {
            debug_print("not enough memory \n");
            os_mutex_unlock( &handle->hmutex );
            return FALSE;
        }

        clt_tmp->sock_fd = sock_fd;
        strcpy( clt_tmp->id, id );
        clt_tmp->hash_value = hash_value;
        clt_tmp->next_sec = get_real_time_sec() + 180; // 120 secs.

        os_mutex_lock( &handle->hmutex );
        if ( !hashtable_insert( handle->hclient, clt_tmp->id, clt_tmp ) )
        {
            debug_print(" hashtable_insert failed \n");
	    free( clt_tmp );
	    os_mutex_unlock( &handle->hmutex );
	    return FALSE;
        }
        list_add_tail( &clt_tmp->list, &handle->head );
        os_mutex_unlock( &handle->hmutex );

    }
    else
    {
        if ( clt->hash_value != hash_value )
        {
            debug_print("re-register id %s \n", id );
            os_mutex_lock( &handle->hmutex );
            list_del( &clt->list );
            hashtable_remove( handle->hclient, clt->id );
            free( clt );
            os_mutex_unlock( &handle->hmutex );

            Device* clt_tmp = NULL;
    	    clt_tmp = ( Device * )calloc( 1, sizeof( Device ) );
            if ( NULL == clt_tmp )
            {
                debug_print("not enough memory \n");
                os_mutex_unlock( &handle->hmutex );
                return FALSE;
            }

            clt_tmp->sock_fd = sock_fd;
            strcpy( clt_tmp->id, id );
            clt_tmp->hash_value = hash_value;
            clt_tmp->next_sec = get_real_time_sec() + 180; // 120 secs.

            os_mutex_lock( &handle->hmutex );
    	    if ( !hashtable_insert( handle->hclient, clt_tmp->id, clt_tmp ) )
    	    {
    	        debug_print(" hashtable_insert failed \n");
                free( clt_tmp );
                os_mutex_unlock( &handle->hmutex );

                return FALSE;
    	    }
            list_add_tail( &clt_tmp->list, &handle->head );
            os_mutex_unlock( &handle->hmutex );
        }
    }

    return TRUE;
}

boolean device_mgr_id_is_valid( DEVICE_MGR_HANDLE handle, int8 *id )
{
    boolean ret = FALSE;
    if ( !id_mgr_id_is_exist( handle->hid_mgr, id ) )
    {
        debug_print( "invalid id [%s][client visit] \n", id );
        return FALSE;
    }
    Device *clt = get_device_by_id( handle, id );
    if ( NULL != clt )
    {
        ret = TRUE;
    }

    return ret;
}

void device_mgr_keep_alive( DEVICE_MGR_HANDLE handle, int8 *id )
{
    Device *clt = get_device_by_id( handle, id );
    if ( NULL != clt )
    {
        clt->next_sec = get_real_time_sec() + 180; // time out is 120 sec.
    }
}

void device_mgr_flush( DEVICE_MGR_HANDLE handle )
{
    Device* clt_list = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    list_for_each_safe( pos, n, &handle->head )
    {
        clt_list = list_entry( pos, Device, list );
        if ( NULL != clt_list )
        {
            if ( get_real_time_sec() >= (int32)clt_list->next_sec )
            {
               debug_print("id %s exit \n", clt_list->id );
               os_mutex_lock( &handle->hmutex );
               list_del( &clt_list->list );
               hashtable_remove( handle->hclient, clt_list->id );
               free( clt_list );
               os_mutex_unlock( &handle->hmutex );
            }
            else
            {
               os_mutex_lock( &handle->hmutex );
               clt_list->next_sec = get_real_time_sec() + 120;
               os_mutex_unlock( &handle->hmutex );
            }
        }
    }
}

boolean device_mgr_remove( DEVICE_MGR_HANDLE handle, int8 *id )
{
    boolean ret = FALSE;
    struct list_head   *pos = NULL;
    struct list_head   *n   = NULL;
    Device* clt_list = NULL;
    Device* clt_tmp = NULL;
	clt_tmp = ( Device* ) hashtable_search( handle->hclient, id );
	if ( NULL == clt_tmp )
	{
		return FALSE;
	}

    list_for_each_safe( pos, n, &handle->head )
    {
        clt_list = list_entry( pos, Device, list );
        if ( NULL != clt_list )
        {
            if ( 0 == strcmp( id, clt_list->id )  )
            {
                os_mutex_lock( &handle->hmutex );
                list_del( &clt_list->list );
                free( clt_list );
                os_mutex_unlock( &handle->hmutex );
                break;
            }
        }
    }

	if ( 0 == hashtable_remove( handle->hclient, id ) )
	{
		ret = TRUE;
	}

	return ret;
}


int32 device_mgr_get_count( DEVICE_MGR_HANDLE handle )
{
	return (int32)hashtable_count( handle->hclient );
}

boolean device_mgr_get_sock_fd( DEVICE_MGR_HANDLE handle, int8 *id, int32 *sock_fd )
{
    boolean ret = FALSE;
    Device *client = ( Device * )hashtable_search( handle->hclient, id );
    if ( NULL != client )
    {
        *sock_fd = client->sock_fd;
        ret = TRUE;
    }

	return ret;
}

void device_mgr_dump( DEVICE_MGR_HANDLE handle )
{
    int32 count;
    int8  buffer[64];
    int8  commom_time[32];
    int8  ymd_buf[16];
    int8  filename[32];

    count = ( int32 )hashtable_count( handle->hclient );

    debug_print("device info: \n");
    debug_print("total [%d] device registered \n", count );

    memset( filename, 0, sizeof( filename ) );
    memset( ymd_buf, 0, sizeof( ymd_buf ) );
    get_time_ymd( ymd_buf );
    sprintf( filename, "%s.log", ymd_buf );

    FILE *fp = fopen( filename, "a+" );
    memset( buffer, 0, sizeof( buffer ) );
    memset( commom_time, 0, sizeof( commom_time ) );
    get_common_time( commom_time );
    sprintf( buffer, "%s - count:[%d]\r\n", commom_time, count  );

    if ( NULL != fp )
    {
        fwrite( buffer, 1, strlen( buffer ), fp );
        fclose( fp );
    }
}

void device_mgr_destroy( DEVICE_MGR_HANDLE handle )
{
    Device* clt_list = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    list_for_each_safe( pos, n, &handle->head )
    {
        clt_list = list_entry( pos, Device, list );
        if ( NULL != clt_list )
        {
            list_del( &clt_list->list );
            free( clt_list );
        }
    }
    list_del( &handle->head );
    id_mgr_destroy( handle->hid_mgr );
    hashtable_destroy( handle->hclient, 0 );
    os_mutex_close( &handle->hmutex );
    free( handle );
}

/************************************************************************************************************************/
/*			Private Function Definition																					*/
/************************************************************************************************************************/
static Device *get_device_by_id( DEVICE_MGR_HANDLE handle, int8 *id )
{
	return ( Device* ) hashtable_search( handle->hclient, id );
}

static unsigned int to_hash ( void* pString )
{
	unsigned int nSeed1, nSeed2;

	char* pKey = (char*)pString;
	char  ch;

	// LOL, coder joke: Dead Code ;)
	nSeed1 = 0xDEADC0DE;
	nSeed2 = 0x7FED7FED;

	while (*pKey != 0)
	{
		ch = toupper (*pKey++);

		// if you changed the size of the g_lCryptTable, you must change the & 0xFF below
		// to & whatever if it's a power of two, or % whatever, if it's not
		nSeed1 = g_lCryptTable[((TYPE << 8) + ch)&0xFF] ^ (nSeed1 + nSeed2);
		nSeed2 = ch + nSeed1 + nSeed2 + (nSeed2 << 5) + 3;
	}

	return nSeed1;
}

static int keys_equal_func ( void *pKey1, void *pKey2 )
{
    return 1;
#if 0
	if ( strcmp((char*)pKey1, (char*)pKey2) == 0 )
    {
		return 1;
	}
	else
    {
		return 0;
	}
#endif
}
