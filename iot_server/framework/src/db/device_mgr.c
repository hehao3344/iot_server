#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "hash_value.h"
#include "sqlite3.h"
#include "group_manage.h"

#define CLIENT_DB               "client.db"
#define DEVICE_DB               "device.db"

#define MAX_ID_NUMBER           (20000)

//char      max 8000 char
//varcahr   var char max 8000 char
//nchar     unicode, max 4000
//nvarchar  var unicode, max 4000

// "create table client_table( sn integer primary key autoincrement, sn nchar(20) )"
// sn: telephone num.

// device: mac address
#define CREATE_CLIENT_STRING        "create table client_table( id integer primary key autoincrement, sn varchar(11), home_group nvarchar(32), email varchar(64), password varchar(32), random char(6), dev_list varchar(256) )"
#define CREATE_INDEX_ID_STRING      "create index id_index on client_table( sn )"
#define CREATE_INDEX_DEV_STRING     "create index id_index on client_table( device )" // deleted it?

#define CREATE_DEVICE_STRING        "create table device_table( id integer primary key autoincrement, mac char(12), name varchar(32), version nvarchar(8), on_off char(2), pm25 varchar(8), filter_time varchar (8) )"
#define CREATE_INDEX_MAC_STRING     "create index id_index on client_table( mac )"

typedef struct GroupMgrObject
{
    // use sqlite3 library.
    sqlite3* client_db;         // sn table;
    int max_client;

    sqlite3* device_db;         // sn table;
    int max_device;

    MUTEX_HANDLE mutex;
} GroupMgrObject;


static GROUP_MGR_HANDLE group_mgr_instance( void );
static int client_get_random_from_db( GROUP_MGR_HANDLE handle, int8 * sn, int8 * random_out );
static int client_sn_is_exist( GROUP_MGR_HANDLE handle, int8 * sn );
static int client_sn_registed_already( GROUP_MGR_HANDLE handle, int8 * sn );
static int client_get_dev_list( GROUP_MGR_HANDLE handle, int8 *sn, int8 *dev_list_out );
static int client_add_dev( GROUP_MGR_HANDLE handle, int8 * sn, int8 * mac );
static int client_add_dev_in_tail( GROUP_MGR_HANDLE handle, int8 *dev_list, int8 *sn, int8 * mac );
static int client_del_dev( GROUP_MGR_HANDLE handle, int8 *dev_list, int8 *sn, int8 * mac );

static int device_mac_is_exist( GROUP_MGR_HANDLE handle, int8 *mac );
static int device_del( GROUP_MGR_HANDLE handle, int8 * mac );

GROUP_MGR_HANDLE group_mgr_create( void )
{
    int result = 0;
    int8* errmsg = NULL;

    GroupMgrObject *handle = group_mgr_instance();
    if ( NULL == handle )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    debug_print("open %s ... \n", CLIENT_DB );
    // create event index database.
    result = sqlite3_open( CLIENT_DB, &handle->client_db );
    if ( SQLITE_OK != result )
    {
        debug_print( "open %s failed \n", CLIENT_DB );
        free( handle );
        return NULL;
    }
    result = sqlite3_exec( handle->client_db, CREATE_CLIENT_STRING, NULL, NULL, &errmsg );
    if ( SQLITE_OK != result )
    {
        //debug_error( "sn db exist already \n" );
        //free( handle );
        //return NULL;
    }
    result = sqlite3_exec( handle->client_db, CREATE_INDEX_ID_STRING, NULL, NULL, &errmsg );
    if ( SQLITE_OK != result )
    {
        //debug_error("warning: create index[exist] \n" );
        //free( handle );
        //return NULL;
    }
    result = sqlite3_exec( handle->client_db, CREATE_INDEX_DEV_STRING, NULL, NULL, &errmsg );
    if ( SQLITE_OK != result )
    {
        //debug_error( "warning: create index[exist] \n" );
        //free( handle );
        //return NULL;
    }

    //device db create
    debug_print("open %s ... \n", DEVICE_DB );
    // create event index database.
    result = sqlite3_open( DEVICE_DB, &handle->device_db );
    if ( SQLITE_OK != result )
    {
        debug_print( "open %s failed \n", DEVICE_DB );
        free( handle );
        return NULL;
    }
    result = sqlite3_exec( handle->device_db, CREATE_DEVICE_STRING, NULL, NULL, &errmsg );
    if ( SQLITE_OK != result )
    {
        //debug_error( "sn db exist already \n" );
        //free( handle );
        //return NULL;
    }
    result = sqlite3_exec( handle->device_db, CREATE_INDEX_MAC_STRING, NULL, NULL, &errmsg );
    if ( SQLITE_OK != result )
    {
        //debug_error("warning: create index[exist] \n" );
        //free( handle );
        //return NULL;
    }

    handle->max_client = MAX_ID_NUMBER;

    os_mutex_open( &handle->mutex, NULL );

    return handle;
}


// we should update client/device datebase at the same time.
int group_mgr_add_device( GROUP_MGR_HANDLE handle, int8 *sn, int8 *mac, int8 *dev_name, int8 *version )
{
    int ret = RET_FAILED;
    int result = 0;
    int8* errmsg = NULL;
    int8  buffer[128];

    if ( !client_sn_is_exist( handle, sn ) )
    {
        debug_print("client sn unexist\n");
        return RET_DB_SN_UNEXIST;
    }

    int8  *dev_list = ( int8 * )malloc( 256 );
    if ( NULL == dev_list )
    {
        return RET_MEMORY_ERROR;
    }

    // client operation.
    memset( dev_list, 0, 256 );
    if ( !client_get_dev_list( handle, sn, dev_list ) )
    {
        // no mac node in client, add mac in device here.
        if ( !client_add_dev( handle, sn, mac ) )
        {
            free( dev_list );
            return RET_DB_SAVE_FAILED;
        }
    }
    else
    {
        if ( !client_add_dev_in_tail( handle, dev_list, sn, mac ) )
        {
            free( dev_list );
            return RET_DB_SAVE_FAILED;
        }
    }
    free( dev_list );

    // device operation.
    if ( !device_mac_is_exist( handle, mac ) )
    {
        // insert
        memset( buffer, 0, sizeof( buffer ) );
        sprintf( buffer, "insert into device_table( mac, name, version ) values ( '%s', '%s', '%s' )", mac, dev_name, version );

        os_mutex_lock( &handle->mutex );
        result = sqlite3_exec( handle->device_db, buffer, 0, 0, &errmsg );
        os_mutex_unlock( &handle->mutex );

        if ( SQLITE_OK == result )
        {
            debug_print("insert dev mac %s success \n", mac );
            ret = RET_SUCCESS;
        }
        else
        {
            debug_error("insert dev mac %s failed \n", mac );
        }
    }
    else
    {
        // update
        memset( buffer, 0, sizeof( buffer ) );
        sprintf( buffer, "update device_table set name='%s', version='%s' where mac = '%s'", dev_name, version, mac );

        os_mutex_lock( &handle->mutex );
        result = sqlite3_exec( handle->device_db, buffer, 0, 0, &errmsg );
        os_mutex_unlock( &handle->mutex );

        if ( SQLITE_OK == result )
        {
            debug_print("update dev_name %s mac %s success \n", dev_name, mac );
            ret = RET_SUCCESS;
        }
        else
        {
            debug_error("update dev mac %s failed \n", mac );
        }
    }

    return ret;
}

// delete db in client/device.
int group_mgr_del_device( GROUP_MGR_HANDLE handle, int8 *sn, int8 *mac )
{
    if ( !client_sn_is_exist( handle, sn ) )
    {
        debug_print("client sn unexist\n");
        return RET_DB_SN_UNEXIST;
    }

    int8  *dev_list = ( int8 * )malloc( 256 );
    if ( NULL == dev_list )
    {
        return RET_MEMORY_ERROR;
    }

    // client operation.
    memset( dev_list, 0, 256 );
    if ( client_get_dev_list( handle, sn, dev_list ) )
    {
        // |001122334455|001122334450
        if ( !client_del_dev( handle, dev_list, sn, mac ) )
        {
            free( dev_list );
            return RET_DB_SAVE_FAILED;
        }
    }
    free( dev_list );

    // del mac section in client dev_list section.
    if ( device_mac_is_exist( handle, mac ) )
    {
        // delete mac in device db.
        if ( device_del( handle, mac ) )
        {
            debug_print("delete %s success \n", mac );
        }
    }
    else
    {
        debug_print("strange no mac  %s ?\n", mac );
    }

    return RET_SUCCESS;
}

// max mac array is 16.
// return the count of mac array.
int group_mgr_device_get_param( GROUP_MGR_HANDLE handle, int8 *mac, DeviceParam *param )
{
    int ret = FALSE;
    int i,j;
    int result = 0;
    int8* errmsg = NULL;

    int8 buffer[128];
    int8 **dbResult;
    int nRow, nColumn;

    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "select * from device_table where mac = '%s'", mac );
    result = sqlite3_get_table( handle->device_db, buffer, &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        for( i=0; i<nRow; i++ )
        {
            for( j=0; j<nColumn; j++ )
            {
                if ( 0 == strcmp( dbResult[j], "name" ) )
                {
                    strcpy( param->name, dbResult[nColumn + j] );
                }
                else if ( 0 == strcmp( dbResult[j], "version" ) )
                {
                    strcpy( param->version, dbResult[nColumn + j] );
                }
                else if ( 0 == strcmp( dbResult[j], "on_off" ) )
                {
                    strcpy( param->on_off, dbResult[nColumn + j] );
                }
                else if ( 0 == strcmp( dbResult[j], "pm25" ) )
                {
                    strcpy( param->pm25, dbResult[nColumn + j] );
                }
                else if ( 0 == strcmp( dbResult[j], "filter_time" ) )
                {
                    strcpy( param->filter_time, dbResult[nColumn + j] );
                }
            }
        }
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            ret = TRUE;
        }
    }

    sqlite3_free_table( dbResult );

    return ret;
}

void group_mgr_destroy( GROUP_MGR_HANDLE handle )
{
    if ( NULL != handle )
    {
        os_mutex_close( &handle->mutex );
        sqlite3_close( handle->client_db );
        sqlite3_close( handle->device_db );
        free( handle );
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
// static function
/////////////////////////////////////////////////////////////////////////////////////////////
static GroupMgrObject* group_mgr_instance()
{
    static GroupMgrObject* handle = NULL;
    if ( NULL == handle )
    {
        handle = ( GroupMgrObject * )calloc( 1, sizeof( GroupMgrObject ) );
    }

    return handle;
}

static int device_mac_is_exist( GROUP_MGR_HANDLE handle, int8 *mac )
{
    int ret = FALSE;
    int result = 0;
    int8* errmsg = NULL;

    int8 buffer[80];
    int8 **dbResult;
    int nRow, nColumn;

    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "select mac from device_table where mac = '%s'", mac );
    result = sqlite3_get_table( handle->device_db, buffer, &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            ret = TRUE;
        }
    }
    sqlite3_free_table( dbResult );

    return ret;
}

static int client_get_dev_list( GROUP_MGR_HANDLE handle, int8 *sn, int8 *dev_list_out )
{
    int ret = FALSE;
    int result = 0;
    int8* errmsg = NULL;

    int8 buffer[128];
    int8 **dbResult;
    int nRow, nColumn;

    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "select dev_list from client_table where sn = '%s'", sn );
    result = sqlite3_get_table( handle->client_db, buffer, &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            if ( NULL != dbResult[nRow] )
            {
                // |001122334455 13*16 = 208
                if ( ( strlen( dbResult[nRow] ) > 0 ) && ( strlen( dbResult[nRow] ) < 256 ) )
                {
                    strcpy( dev_list_out, dbResult[nRow] );
                    debug_print("get device list %s \n", dev_list_out );
                    ret = TRUE;
                }
            }
        }
    }
    sqlite3_free_table( dbResult );

    return ret;
}

static int client_add_dev( GROUP_MGR_HANDLE handle, int8 * sn, int8 * mac )
{
    int ret = FALSE;

    int result = 0;
    int8* errmsg = NULL;
    int8 buffer[80];

    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "update client_table set dev_list='|%s' where sn = '%s'", mac, sn );

    os_mutex_lock( &handle->mutex );
    result = sqlite3_exec( handle->client_db, buffer, 0, 0, &errmsg );
    os_mutex_unlock( &handle->mutex );

    if ( SQLITE_OK == result )
    {
        debug_print("add client mac %s success \n", mac );
        ret = TRUE;
    }
    else
    {
        debug_error("add client mac %s failed \n", mac );
    }

    return ret;
}

static int client_add_dev_in_tail( GROUP_MGR_HANDLE handle, int8 *dev_list, int8 *sn, int8 * mac )
{
    int ret = FALSE;

    int result = 0;
    int8* errmsg = NULL;
    int8  *buffer= NULL;

    if ( NULL != strstr( dev_list, mac ) )
    {
        debug_print("mac %s %s exist already \n", dev_list, mac );
        return TRUE;
    }
    if ( strlen( dev_list ) >= 13*16 )
    {
        debug_error("db full already, please delete at first \n" );
        return FALSE;
    }

    // mac unexist, we add it in tail.
    int8 * new_dev_list = ( int8 * )malloc( 256 );
    if ( NULL == new_dev_list )
    {
        debug_error("malloc failed \n");
        return FALSE;
    }
    sprintf( new_dev_list, "%s|%s", dev_list, mac );

    buffer = ( int8 * )malloc( 320 ); // 320 is enough.
    if ( NULL == buffer )
    {
        debug_error("malloc failed \n");
        free( new_dev_list );
        return FALSE;
    }

    memset( buffer, 0, 320 );
    sprintf( buffer, "update client_table set dev_list='%s' where sn = '%s'", new_dev_list, sn );

    os_mutex_lock( &handle->mutex );
    result = sqlite3_exec( handle->client_db, buffer, 0, 0, &errmsg );
    os_mutex_unlock( &handle->mutex );

    if ( SQLITE_OK == result )
    {
        debug_print("insert mac %s success \n", mac );
        ret = TRUE;
    }
    else
    {
        debug_error("insert mac %s failed \n", mac );
    }

    free( buffer );
    free( new_dev_list );

    return ret;
}

static int client_del_dev( GROUP_MGR_HANDLE handle, int8 *dev_list, int8 *sn, int8 * mac )
{
    int ret = FALSE;

    int result = 0;
    int8* errmsg = NULL;
    int8  *buffer= NULL;

    int8 *head, *tail;
    head = tail = NULL;

    // no mac string in dev_list.
    head = strstr( dev_list, mac );

    if ( NULL == head )
    {
        debug_print("mac %s unexist \n", mac );
        return TRUE;
    }

    if ( ( ( head - dev_list ) < 1 ) && ( ( dev_list - head ) >= 256 ) ) // in fact not 256
    {
        debug_print("strange string \n" );
        return FALSE;
    }

    if ( '|' == *( head - 1 ) )
    {
        head = head - 1;
    }

    if ( '|' == *( head + 13 ) ) // 13 = '|' + mac(12)
    {
        int len = strlen( dev_list );

        memcpy( head, head + 13, strlen( head + 13 ) );
        memset( dev_list + len - 13, 0, 13 );
    }
    else
    {
        memset( head, 0, 13 );
    }

    if ( strlen( dev_list ) >= 13*16 )
    {
        debug_error("db full already, please delete at first \n" );
        return FALSE;
    }


    buffer = ( int8 * )malloc( 320 ); // 320 is enough.
    if ( NULL == buffer )
    {
        debug_error("malloc failed \n");
        return FALSE;
    }

    memset( buffer, 0, 320 );
    sprintf( buffer, "update client_table set dev_list='%s' where sn = '%s'", dev_list, sn );

    os_mutex_lock( &handle->mutex );
    result = sqlite3_exec( handle->client_db, buffer, 0, 0, &errmsg );
    os_mutex_unlock( &handle->mutex );

    if ( SQLITE_OK == result )
    {
        debug_print("update mac %s success \n", mac );
        ret = TRUE;
    }
    else
    {
        debug_error("update mac %s failed \n", mac );
    }

    free( buffer );

    return ret;
}

static int client_get_random_from_db( GROUP_MGR_HANDLE handle, int8 * sn, int8 * random_out )
{
    int ret = FALSE;
    int result = 0;
    int8* errmsg = NULL;

    int8 buffer[128];
    int8 **dbResult;
    int nRow, nColumn;

    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "select random from client_table where sn = '%s'", sn );
    result = sqlite3_get_table( handle->client_db, buffer, &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            if ( NULL != dbResult[nRow] )
            {
                if ( 6 == strlen( dbResult[nRow] ) )
                {
                    strcpy( random_out, dbResult[nRow] );
                    //debug_print("get random %s \n", random_out );
                    ret = TRUE;
                }
            }
        }
    }
    sqlite3_free_table( dbResult );

    return ret;
}

static int device_del( GROUP_MGR_HANDLE handle, int8 * mac )
{
    int8 buffer[128];
    int result = 0;
    sqlite3_stmt *stmt = NULL;
    const char * errorCode = NULL;

    // delete the first item here.
    sqlite3_exec( handle->device_db, "begin", 0, 0, 0 );

    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "delete from device_table  where mac = '%s'", mac );

    os_mutex_lock( &handle->mutex );
    result = sqlite3_prepare( handle->device_db, buffer, -1, &stmt, &errorCode );
    if ( result != SQLITE_OK )
    {
        debug_error( "result Code:%d error message:%s", result, errorCode);
        sqlite3_finalize( stmt );
        os_mutex_unlock(&handle->mutex);
        return FALSE;
    }

    result = sqlite3_step(stmt);
    if ( result!= SQLITE_DONE )
    {
        debug_print( "Delete failed! Result Code:%d  ErrorMessage:%s", result, errorCode);
        sqlite3_finalize( stmt );
        os_mutex_unlock(&handle->mutex);
        return FALSE;
    }
    os_mutex_unlock( &handle->mutex );

    sqlite3_finalize( stmt );
    sqlite3_exec( handle->device_db, "commit", 0, 0, 0 );

    return TRUE;
}

// 1000 - 140ms.
int group_mgr_unit_test( void )
{
    GROUP_MGR_HANDLE handle = group_mgr_create();

    int8 random_out[8];
    memset( random_out, 0, sizeof( random_out ) );

    group_mgr_client_get_random( handle, "15602943003", random_out );

    group_mgr_client_register( handle, "15602943003", "home",
                               "hehao3344@126.com", "admin", random_out );

    //int8 password_out[32];
    //memset( password_out, 0, sizeof( password_out ) );

    //if ( group_mgr_client_get_password( handle, "15602943003", password_out ) )
    //{
    //    debug_print("get password %s \n", password_out );
    //}

    //group_mgr_add_device( handle, "15602943003", "001122334451", "my_home1", "1001" );
    //group_mgr_add_device( handle, "15602943003", "001122334452", "my_home2", "1002" );
    //group_mgr_add_device( handle, "15602943003", "001122334453", "my_home3", "1003" );
    //group_mgr_add_device( handle, "15602943003", "001122334454", "my_home4", "1004" );
    //group_mgr_add_device( handle, "15602943003", "001122334455", "my_home5", "1005" );
    //group_mgr_add_device( handle, "15602943003", "001122334456", "my_home6", "1006" );


    if ( RET_SUCCESS == group_mgr_del_device( handle, "15602943003", "001122334455" ) )
    {
        debug_print("del mac success \n");
    }
    if ( RET_SUCCESS == group_mgr_del_device( handle, "15602943003", "001122334453" ) )
    {
        debug_print("del mac success \n");
    }

    int i;
    int8 buf[156] = { 0 };
    int8 *mac[16];

    for ( i=0; i<16; i++ )
    {
        mac[i] = &buf[i*16];
    }
    DeviceParam dev_param;

    int count = group_mgr_client_get_device_mac( handle, "15602943003", mac );
    for( i=0; i<count; i++ )
    {
        printf("i=%d mac = %s \n", i, mac[i]);

        memset( &dev_param, 0, sizeof( DeviceParam ) );
        if ( group_mgr_device_get_param( handle, mac[i], &dev_param ) )
        {
            printf("get name %s version %s \n", dev_param.name, dev_param.version );
        }
    }


    return 0;

#if 0
    int i;
    long long start = get_real_time_usec();

    //group_mgr_generate_id_db( handle );
    //return 0;

    int8 num[12];
    for( i=0; i<10000; i++ )
    {
        memset( num, 0, 12 );
        sprintf( num, "1560294%04d", i );
        if ( group_mgr_client_get_random( handle, num, random_out ) )
        {
            //printf("get random %s \n", random_out );
        }

        // group_mgr_id_is_exist( handle, "MCSZC598CB6AC59A2767" );
    }

    printf( "use %lld \n", get_real_time_usec() - start );

    group_mgr_destroy( handle );

    return 0;
#endif
}
