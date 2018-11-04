#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "component/sqlite3.h"
#include "file_index_mgr.h"

#ifdef WIN32
#define TIME_INDEX_DB_NAME      "./time_index_table.db"
#define TRIGGER_INDEX_DB_NAME   "./trigger_index_table.db"
#else
#define TIME_INDEX_DB_NAME      "/mnt/cf/time_index_table.db"
#define TRIGGER_INDEX_DB_NAME   "/mnt/cf/trigger_index_table.db"
#endif

#define DEFAULT_MAX_ITEM_NUMBER 10 // only for test

#define CREATE_TIME_INDEX_SECTION_STRING    "create table TimeIndexTable( ID INTEGER primary key autoincrement, stime nchar(24), etime nchar(24), filename nvarchar(32), offset INTEGER )"
#define CREATE_EVENT_INDEX_SECTION_STRING   "create table EventIndexTable( ID INTEGER primary key autoincrement, stime nchar(24), etime nchar(24), filename nvarchar(32), eflags INTEGER )"

typedef struct FileIndexMgrObject
{
    // use sqlite3 library.
    sqlite3* timeDb;    // time table;
    sqlite3* evtDb;     // trigger table;

    int32 maxTimeIndexItem;
    int32 maxEventIndexItem;
    boolean timeIndexUnClose;

    MUTEX_HANDLE mutex;

} FileIndexMgrObject;

static boolean  create_dir( const char *path );

static FINDEX_MGR_HANDLE file_index_mgr_instance( void );
static boolean delete_item_from_event_index_table( FINDEX_MGR_HANDLE handle, int8 *stime );
static boolean compare_time_interval( int8 *stime1, int8 *etime1, int8 * stime2, int8 * etime2 );
static boolean compare_time_interval2( int8 *stime1, int8 *etime1, int8 * stime2, int8 * etime2 );
static int32   get_time_index_count( FINDEX_MGR_HANDLE handle );
static int32   get_time_index_first_id( FINDEX_MGR_HANDLE handle );
static int32   get_time_index_last_id( FINDEX_MGR_HANDLE handle );
static int32   get_event_index_count( FINDEX_MGR_HANDLE handle );
static int32   get_event_index_last_id( FINDEX_MGR_HANDLE handle );
static int32   get_seconds( int8 *strTime );
static boolean get_time_index_file_param( FINDEX_MGR_HANDLE handle, int32 id, int8 *fileName, int32 *offset );
static boolean file_index_mgr_get_time_index_id_zone( FINDEX_MGR_HANDLE handle, int32 *startID, int32 *endID );
static boolean file_index_mgr_get_time_index( FINDEX_MGR_HANDLE handle, int32 id, int8 *stime, int8 *etime );

FINDEX_MGR_HANDLE file_index_mgr_create( int8 *timeIndexDB, int8 *eventIndexDB )
{
    int32 result = 0;
    int8* errmsg = NULL;

    FileIndexMgrObject *fimo = file_index_mgr_instance();
    if ( NULL == fimo )
    {
        return NULL;
    }

    debug_print("Open %s %s \n", timeIndexDB, eventIndexDB );

    // create event index database.
    result = sqlite3_open( timeIndexDB, &fimo->timeDb );
    if ( SQLITE_OK != result )
    {
        debug_print( "Open %s failed \n", timeIndexDB );
        free( fimo );
        return NULL;
    }

    result = sqlite3_exec( fimo->timeDb, CREATE_TIME_INDEX_SECTION_STRING, NULL, NULL, &errmsg );
    if ( SQLITE_OK != result )
    {
        debug_print( "table TimeIndexTable exist already \n" );

        int8 **dbResult;
        int32 nRow, nColumn;
        int8 stime[32];
        int8 etime[32];

        memset( stime, 0, sizeof( stime ) );
        // get start time here. dbResult
        result = sqlite3_get_table( fimo->timeDb, "select stime from TimeIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
        if ( SQLITE_OK == result )
        {
            debug_print("%d - %d \n", nRow, nColumn);
            if ( ( nRow > 0 ) && ( nColumn > 0 ) )
            {
                strcpy( stime, dbResult[nRow] );
            }
        }
        sqlite3_free_table( dbResult ); // free, so we should save stime and etime.

        // get start time here. dbResult
        memset( etime, 0, sizeof( etime ) );
        result = sqlite3_get_table( fimo->timeDb, "select etime from TimeIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
        if ( SQLITE_OK == result )
        {
            if ( ( nRow > 0 ) && ( nColumn > 0 ) )
            {
                strcpy( etime, dbResult[nRow] );
            }
        }
        sqlite3_free_table( dbResult );

        debug_print( "the last start-end time [%s][%s] \n", stime, etime );

        if ( ( NULL != stime ) && ( NULL != etime ) )
        {
            if ( 0 == strcmp( stime, etime ) )
            {
                fimo->timeIndexUnClose = TRUE;
            }
        }
    }

    fimo->maxTimeIndexItem = DEFAULT_MAX_ITEM_NUMBER;

    // create event index database.
    result = sqlite3_open( eventIndexDB, &fimo->evtDb );
    if ( SQLITE_OK != result )
    {
        debug_print( "Open %s failed \n", eventIndexDB );
        free( fimo );
        return NULL;
    }

    result = sqlite3_exec( fimo->evtDb, CREATE_EVENT_INDEX_SECTION_STRING, NULL, NULL, &errmsg );
    if ( SQLITE_OK != result )
    {
        //debug_print( "create table %d %s \n", result, errmsg );
        debug_print( "table EventIndexTable exist already \n" );
    }

    fimo->maxEventIndexItem = DEFAULT_MAX_ITEM_NUMBER;

    os_mutex_open( &fimo->mutex, NULL );

    return fimo;
}

boolean file_index_mgr_time_index_db_close_is_normal( FINDEX_MGR_HANDLE handle )
{
    return handle->timeIndexUnClose ? FALSE : TRUE;
}

boolean file_index_mgr_get_last_start_time( FINDEX_MGR_HANDLE handle, int8 *stime )
{
    boolean ret = FALSE;
    int32 result = 0;
    int8* errmsg = NULL;

    int8 **dbResult;
    int32 nRow, nColumn;

    result = sqlite3_get_table( handle->timeDb, "select stime from TimeIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            strcpy( stime, dbResult[nRow] );
            ret = TRUE;
        }
    }
    sqlite3_free_table( dbResult );

    return ret;
}

void file_index_mgr_destroy( FINDEX_MGR_HANDLE handle )
{
    if ( NULL != handle )
    {
        os_mutex_close( &handle->mutex );
        sqlite3_close( handle->timeDb );
        sqlite3_close( handle->evtDb );
        free( handle );
    }
}

// param out1: timeIndexDB
// param out2: offset
boolean file_index_mgr_get_last_time_index_param( FINDEX_MGR_HANDLE handle, int8 *timeIndexDB, int32 *offset )
{
    int32 result = 0;
    int32 curOffset = 0;
    int8* errmsg = NULL;

    int8 **dbResult;
    int32 nRow, nColumn;

    // int32 i, j;
    if ( ( NULL == timeIndexDB ) || ( NULL == offset ) )
    {
        return FALSE;
    }
    // get offset here. dbResult
    result = sqlite3_get_table( handle->timeDb, "select offset from TimeIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            curOffset = atoi( dbResult[nRow] );
        }
    }
    if ( curOffset > 0 )
    {
        *offset = curOffset;
    }
    else
    {
        *offset = 0;
    }
    sqlite3_free_table( dbResult );
    // get  here. dbResult
    result = sqlite3_get_table( handle->timeDb, "select filename from TimeIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            strcpy( timeIndexDB, dbResult[nRow] );
        }
    }
    sqlite3_free_table( dbResult );

    return TRUE;
}

boolean file_index_mgr_open_time_index_item( FINDEX_MGR_HANDLE handle, int8 *stime, int8 *fileName, int32 offset )
{
    int32   result = 0;
    boolean ret = FALSE;
    int8* errmsg = NULL;
    int8 buffer[256];
    int32 count = 0;
    count = get_time_index_count( handle );
    if ( count >= handle->maxTimeIndexItem )
    {
        debug_print( "reach the max item number %d, deleting some item\n", count );

        int32 i;
        sqlite3_stmt *stmt = NULL;
        const char * errorCode = NULL;

        int8 **dbResult;
        int32 nRow, nColumn;
        int8  tstime[32], tetime[32];
        int32 id = 0;

        // get start|end time from time index table of deleting item.
        memset( buffer, 0, sizeof( buffer ) );
        id = get_time_index_first_id( handle );
        if ( id <= 0 )
        {
            debug_print("get first id %d \n", id);
            return FALSE;
        }

        sprintf( buffer, "select stime from TimeIndexTable where [ID] = %d", id );
        result = sqlite3_get_table( handle->timeDb, buffer, &dbResult, &nRow, &nColumn, &errmsg );
        if ( SQLITE_OK == result )
        {
            // result store in dbResult[]
            if ( ( nRow > 0 ) && ( nColumn > 0 ) )
            {
                memset( tstime, 0, sizeof( tstime ) );
                strcpy( tstime, dbResult[nRow] );
            }
        }
        sqlite3_free_table( dbResult );

        memset( buffer, 0, sizeof( buffer ) );
        sprintf( buffer, "select etime from TimeIndexTable where [ID] = %d", id );
        result = sqlite3_get_table( handle->timeDb, buffer, &dbResult, &nRow, &nColumn, &errmsg );
        if ( SQLITE_OK == result )
        {
            // result store in dbResult[]
            if ( ( nRow > 0 ) && ( nColumn > 0 ) )
            {
                memset( tetime, 0, sizeof( tetime ) );
                strcpy( tetime, dbResult[nRow] );
            }
        }
        sqlite3_free_table( dbResult );

        // delete the first item here.
        os_mutex_lock(&handle->mutex);
        sqlite3_exec( handle->timeDb, "BEGIN", 0, 0, 0 );
        memset( buffer, 0, sizeof( buffer ) );
        sprintf( buffer, "Delete From TimeIndexTable  where [ID] = %d", id );

        debug_print("Deleting ... %d \n", id );
        result = sqlite3_prepare( handle->timeDb, buffer, -1, &stmt, &errorCode );
        if ( result != SQLITE_OK )
        {
            debug_print( "Result Code:%d ErrorMessage:%s", result, errorCode);
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
        os_mutex_unlock(&handle->mutex);
        debug_print("time db item %d deleted \n", id);

        //handle->curTimeIndexDelItem++;
        //handle->curTimeIndexItem--;
        sqlite3_finalize( stmt );
        sqlite3_exec( handle->timeDb, "COMMIT", 0, 0, 0);

        int32 start, end;
        int8 estime[32], eetime[32];
        result = sqlite3_get_table( handle->evtDb, "select ID from EventIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
        if ( SQLITE_OK == result )
        {
            // result store in dbResult[]
            if ( ( nRow > 0 ) && ( nColumn > 0 ) )
            {
                start = atoi( dbResult[1] );     // 1 is the first ID item.
                end   = atoi( dbResult[nRow] );  // nRow is the last ID item.
            }
        }
        sqlite3_free_table( dbResult );

        for( i=start; i<end; i++ )
        {
            if ( file_index_mgr_get_event( handle, i, estime, eetime ) )
            {
                // delete it.
                if ( compare_time_interval( tstime, tetime, estime, eetime ) )
                {
                    delete_item_from_event_index_table( handle, estime );
                }
            }
        }
    }

    memset( buffer, 0, sizeof( buffer ) );
    // when open the section, etime = stime.
    sprintf( buffer, "insert into TimeIndexTable( stime, etime, filename, offset ) values ( '%s', '%s', '%s', '%d' )",
                        stime, stime, fileName, offset );
    result = sqlite3_exec( handle->timeDb, buffer, 0, 0, &errmsg );
    if ( result == SQLITE_OK )
    {
        debug_print("Insert ID:%d ok, content is %s \n", count, buffer );
        ret = TRUE;
    }
    else
    {
        debug_print("insert failed \n" );
    }
    return ret;
}

boolean file_index_mgr_close_time_index_item( FINDEX_MGR_HANDLE handle, int8 *etime )
{
    boolean ret = FALSE;
    int8* errmsg = NULL;
    int8 buffer[256];
    int32 result = 0;
    int32 id = 0;

    memset( buffer, 0, sizeof( buffer ) );
    id = get_time_index_last_id( handle );

    os_mutex_lock(&handle->mutex);
    sprintf( buffer, "update TimeIndexTable set etime='%s' where ID = %d", etime, id );
    result = sqlite3_exec( handle->timeDb, buffer, 0, 0, &errmsg );
    if ( result == SQLITE_OK )
    {
        debug_print("%s \n", buffer );
        ret = TRUE;
    }
    else
    {
        debug_print("insert failed \n" );
    }
    os_mutex_unlock(&handle->mutex);

    return ret;
}

boolean file_index_mgr_del_time_index_section_by_filename( FINDEX_MGR_HANDLE handle, int8 *fileName )
{
    boolean ret = FALSE;
    int32 result = 0;
    int8 buffer[256];
    memset( buffer, 0, sizeof( buffer ) );

    sqlite3_stmt *stmt = NULL;
    const char * errorCode = NULL;

    // delete the first item here.
    sqlite3_exec( handle->timeDb, "BEGIN", 0, 0, 0 );
    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "Delete From TimeIndexTable  where filename = '%s'", fileName );
    os_mutex_lock(&handle->mutex);
    result = sqlite3_prepare( handle->timeDb, buffer, -1, &stmt, &errorCode );
    if ( result != SQLITE_OK )
    {
        debug_print( "Result Code:%d ErrorMessage:%s", result, errorCode);
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

    debug_print("time db filename %s deleted[time_index] \n", fileName );

    sqlite3_finalize( stmt );
    sqlite3_exec( handle->timeDb, "COMMIT", 0, 0, 0);
    os_mutex_unlock(&handle->mutex);

    return ret;
}

boolean file_index_mgr_del_event_index_section_by_filename( FINDEX_MGR_HANDLE handle, int8 *fileName )
{
    boolean ret = FALSE;
    int32 result = 0;
    int8 buffer[256];
    memset( buffer, 0, sizeof( buffer ) );

    sqlite3_stmt *stmt = NULL;
    const char * errorCode = NULL;

    // delete the first item here.
    sqlite3_exec( handle->evtDb, "BEGIN", 0, 0, 0 );
    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "Delete From EventIndexTable  where filename = '%s'", fileName );
    os_mutex_lock(&handle->mutex);
    result = sqlite3_prepare( handle->evtDb, buffer, -1, &stmt, &errorCode );
    if ( result != SQLITE_OK )
    {
        debug_print( "Result Code:%d ErrorMessage:%s", result, errorCode);
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
    os_mutex_unlock(&handle->mutex);

    debug_print("time db filename %s deleted[event_index] \n", fileName );

    sqlite3_finalize( stmt );
    sqlite3_exec( handle->evtDb, "COMMIT", 0, 0, 0);

    return ret;
}

// get event count.
int32 file_index_mgr_get_event_count( FINDEX_MGR_HANDLE handle )
{
    return get_event_index_count( handle );
}

boolean file_index_mgr_get_event_index_id_zone( FINDEX_MGR_HANDLE handle, int32 *startID, int32 *endID )
{
    boolean ret = FALSE;
    int8* errmsg = NULL;
    int32 nRow, nColumn;
    int32 result = 0;
    int8 **dbResult;

    *startID = *endID = 0;

    result = sqlite3_get_table( handle->evtDb, "select ID from EventIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        // result store in dbResult[]
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            *startID = atoi( dbResult[1] );     // 1 is the first ID item.
            *endID   = atoi( dbResult[nRow] );  // nRow is the last ID item.
            ret = TRUE;
        }
    }
    sqlite3_free_table( dbResult );

    return ret;
}

// param index: in, from 0 to ( event_count - 1 );
//       stime: out start time,
//       etime: out end time.
boolean file_index_mgr_get_event( FINDEX_MGR_HANDLE handle, int32 id, int8 *stime, int8 *etime )
{
    boolean ret = FALSE;
    int32 result = 0;
    int8* errmsg = NULL;

    int8 buffer[128];
    int8 **dbResult;
    int32 nRow, nColumn;

    // int32 i, j;
    if ( ( NULL == stime ) || ( NULL == etime ) )
    {
        debug_print("invalid param \n");
        return FALSE;
    }

    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "select stime from EventIndexTable where [ID] = %d", id );

    // get event flags here. dbResult
    result = sqlite3_get_table( handle->evtDb, buffer, &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            strcpy( stime, dbResult[nRow] );
        }
    }
    sqlite3_free_table( dbResult );

    // get event flags here. dbResult
    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "select etime from EventIndexTable where [ID] = %d",  id  );
    result = sqlite3_get_table( handle->evtDb, buffer, &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            strcpy( etime, dbResult[nRow] );
            ret = TRUE;
        }
    }
    sqlite3_free_table( dbResult );

    return ret;
}

boolean file_index_mgr_open_event( FINDEX_MGR_HANDLE handle, int8 *stime, int8 *filename )
{
    int32   result = 0;
    boolean ret = FALSE;
    int8* errmsg = NULL;
    int8 buffer[256];
    int32 count = 0;

    count = get_event_index_count( handle );
    if ( count >= handle->maxEventIndexItem )
    {
        debug_print( "reach the max item number %d, delete some item\n", count );

        int32 i;
        boolean delSuccess = FALSE;
        int8 estime[32], eetime[32]; // event start time, event end time.

        // delete event index table in event database.
        for( i=0; i<count; i++ )
        {
            memset( estime, 0, sizeof( estime ) );
            memset( eetime, 0, sizeof( eetime ) );
            file_index_mgr_get_event( handle, i, estime, eetime );
            if ( delete_item_from_event_index_table( handle, estime ) )
            {
                delSuccess = TRUE;
                debug_print("delete item ok \n");
                break;
            }
        }

        if ( !delSuccess )
        {
            return FALSE;
        }
    }

    memset( buffer, 0, sizeof( buffer ) );
    // when open the section, etime = stime.
    sprintf( buffer, "insert into EventIndexTable( stime, etime, filename, eflags ) values ( '%s', '%s', '%s', '%d' )",
                     stime, stime, filename, 1 ); // all of eflags is '1';

    result = sqlite3_exec( handle->evtDb, buffer, 0, 0, &errmsg );
    if ( result == SQLITE_OK )
    {
        debug_print("%s \n", buffer );
        ret = TRUE;
    }
    else
    {
        debug_print("insert failed \n" );
    }

    return TRUE;
}

boolean file_index_mgr_close_event( FINDEX_MGR_HANDLE handle, int8 *etime )
{
    boolean ret = FALSE;
    int8* errmsg = NULL;
    int8 buffer[256];
    int32 result = 0;
    int32 id = 0;
    memset( buffer, 0, sizeof( buffer ) );
    id = get_event_index_last_id( handle );
    // id is last id.
    sprintf( buffer, "update EventIndexTable set etime='%s' where ID = %d", etime, id );
    os_mutex_lock(&handle->mutex);
    result = sqlite3_exec( handle->evtDb, buffer, 0, 0, &errmsg );
    if ( result == SQLITE_OK )
    {
        debug_print("%s \n", buffer );
        ret = TRUE;
    }
    else
    {
        debug_print("insert failed \n" );
    }
    os_mutex_unlock(&handle->mutex);

    return ret;
}

// get offset and filename according to event ID.
boolean file_index_mgr_get_file_according_to_event_id( FINDEX_MGR_HANDLE handle, int32 id, int8 *fileName, int32 *offset )
{
    boolean ret = FALSE;
    int8 estime[32], eetime[32];
    int8 tstime[32], tetime[32];

    memset( estime, 0, sizeof( estime ) );
    memset( eetime, 0, sizeof( eetime ) );
    // get start and end time form event table.
    if ( file_index_mgr_get_event( handle, id, estime, eetime ) )
    {
        int32 i;
        int32 startID = 0, endID = 0;
        if ( file_index_mgr_get_time_index_id_zone( handle, &startID, &endID ) )
        {
            for( i=startID; i<=endID; i++ )
            {
                memset( tstime, 0, sizeof( tstime ) );
                memset( tetime, 0, sizeof( tetime ) );
                if ( file_index_mgr_get_time_index( handle, i, tstime, tetime ) )
                {
                    // debug_print("time index id: %d, event %s %s time %s %s\n", i, estime, eetime, tstime, tetime);
                    if ( compare_time_interval2( tstime, tetime, estime, eetime ) )
                    {
                        debug_print("event [%s %s] is in [%s %s] \n", estime, eetime, tstime, tetime);

                        if( get_time_index_file_param( handle, i, fileName, offset ) )
                        {
                            debug_print("get file param %s:%d \n", fileName, *offset );
                            ret = TRUE;
                        }
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

void file_index_mgr_dump_time_index( FINDEX_MGR_HANDLE handle )
{
    int32 i;
    boolean ret = FALSE;
    int8 fileName[32];
    int8 tstime[32], tetime[32];
    int32 startID = 0, endID = 0;
    int32 offset = 0;

    if ( file_index_mgr_get_time_index_id_zone( handle, &startID, &endID ) )
    {
        for ( i=startID; i<=endID; i++ )
        {
            memset( tstime, 0, sizeof( tstime ) );
            memset( tetime, 0, sizeof( tetime ) );
            if ( file_index_mgr_get_time_index( handle, i, tstime, tetime ) )
            {
                debug_print("id: %d, time %s %s, ", i, tstime, tetime );
            }
            memset( fileName, 0, sizeof( fileName ) );
            if( get_time_index_file_param( handle, i, fileName, &offset ) )
            {
                debug_print("filename %s offset %d \n", fileName, offset );
                ret = TRUE;
            }
        }
    }


#if 0
    int32 result = 0;
    int8* errmsg = NULL;

    int8 **dbResult;
    int32 nRow, nColumn;
    int32 i;

    // get offset here. dbResult
    result = sqlite3_get_table( handle->timeDb, "select stime from TimeIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        for( i=1; i<(nRow+1); i++ )
        {
            debug_print("%s %s \n", dbResult[0], dbResult[i] );

            //for( j=0; j<nColumn; j++ )
            //{
            //    debug_print("%s %s \n", dbResult[0], dbResult[i] );
            //}
        }
    }
    sqlite3_free_table( dbResult );

    // get offset here. dbResult
    result = sqlite3_get_table( handle->timeDb, "select etime from TimeIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        for( i=1; i<(nRow+1); i++ )
        {
            debug_print("%s %s \n", dbResult[0], dbResult[i] );

            //for( j=0; j<nColumn; j++ )
            //{
            //    debug_print("%s %s \n", dbResult[0], dbResult[i] );
            //}
        }
    }
    sqlite3_free_table( dbResult );
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
// static function
/////////////////////////////////////////////////////////////////////////////////////////////
static boolean create_dir( const char *path )
{
#ifdef WIN32

#else
    int32 i, length;
    int8 dirName[128];
    memset( dirName, 0, sizeof( dirName ) );
    strcpy( dirName, path );

    length = strlen(dirName);
    if ( '/' != dirName[length-1] )
    {
        strcat(dirName, "/");
    }

    length = strlen(dirName);

    for( i=1; i<length; i++ )
    {
        if ( dirName[i]=='/' )
        {
            dirName[i] = 0;
            if ( 0 != access( dirName, F_OK ) )
            {
                if ( -1 == mkdir( dirName, 0755 ) )
                {
                      debug_print(  "mkdir");
                      return FALSE;
                }
            }
            dirName[i] = '/';
        }
    }
#endif

    return TRUE;
}

static boolean get_time_index_file_param( FINDEX_MGR_HANDLE handle, int32 id, int8 *fileName, int32 *offset )
{
    boolean ret = FALSE;
    int32 result = 0;
    int32 correctCount = 0;
    int8* errmsg = NULL;

    int8 buffer[128];
    int8 **dbResult;
    int32 nRow, nColumn;

    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "select filename from TimeIndexTable where [ID] = %d", id );

    // get event flags here. dbResult
    result = sqlite3_get_table( handle->timeDb, buffer, &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            strcpy( fileName, dbResult[nRow] );
            correctCount++;
        }
    }
    sqlite3_free_table( dbResult );


    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "select offset from TimeIndexTable where [ID] = %d", id );

    // get event flags here. dbResult
    *offset = 0;
    result = sqlite3_get_table( handle->timeDb, buffer, &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            *offset = atoi( dbResult[nRow] );
            correctCount++;
        }
    }
    sqlite3_free_table( dbResult );

    if ( 2 == correctCount )
    {
        ret = TRUE;
    }

    return ret;
}

static boolean delete_item_from_event_index_table( FINDEX_MGR_HANDLE handle, int8 *stime )
{
    sqlite3_stmt *stmt = NULL;
    const char *errorCode = NULL;
    int8 buffer[256];
    int32 result = 0;

    // delete the first item here.
    sqlite3_exec( handle->evtDb, "BEGIN", 0, 0, 0 );
    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "Delete From EventIndexTable  where stime='%s'", stime );

    debug_print("deleting ... %s \n", stime );
    os_mutex_lock(&handle->mutex);
    result = sqlite3_prepare( handle->evtDb, buffer, -1, &stmt, &errorCode );
    if ( result != SQLITE_OK )
    {
        debug_print( "Result Code:%d ErrorMessage:%s", result, errorCode);
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

    debug_print("event db item %s deleted \n", stime );
    // handle->curEventIndexItem--;

    sqlite3_finalize( stmt );
    sqlite3_exec( handle->evtDb, "COMMIT", 0, 0, 0);
    os_mutex_unlock(&handle->mutex);

    return TRUE;
}

static boolean file_index_mgr_get_time_index_id_zone( FINDEX_MGR_HANDLE handle, int32 *startID, int32 *endID )
{
    boolean ret = FALSE;
    int8* errmsg = NULL;
    int32 nRow, nColumn;
    int32 result = 0;
    int8 **dbResult;

    *startID = *endID = 0;
    result = sqlite3_get_table( handle->timeDb, "select ID from TimeIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        // result store in dbResult[]
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            *startID = atoi( dbResult[1] );     // 1 is the first ID item.
            *endID   = atoi( dbResult[nRow] );  // nRow is the last ID item.
            ret = TRUE;
        }
    }
    sqlite3_free_table( dbResult );

    return ret;
}

// param index: in, from 0 to ( event_count - 1 );
//       stime: out start time,
//       etime: out end time.
static boolean file_index_mgr_get_time_index( FINDEX_MGR_HANDLE handle, int32 id, int8 *stime, int8 *etime )
{
    boolean ret = FALSE;
    int32 result = 0;
    int8* errmsg = NULL;

    int8 buffer[128];
    int8 **dbResult;
    int32 nRow, nColumn;

    // int32 i, j;
    if ( ( NULL == stime ) || ( NULL == etime ) )
    {
        return FALSE;
    }

    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "select stime from TimeIndexTable where [ID] = %d", id );

    // get event flags here. dbResult
    result = sqlite3_get_table( handle->timeDb, buffer, &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            strcpy( stime, dbResult[nRow] );
        }
    }
    sqlite3_free_table( dbResult );

    // get event flags here. dbResult
    memset( buffer, 0, sizeof( buffer ) );
    sprintf( buffer, "select etime from TimeIndexTable where [ID] = %d",  id  );
    result = sqlite3_get_table( handle->timeDb, buffer, &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            strcpy( etime, dbResult[nRow] );
            ret = TRUE;
        }
    }
    sqlite3_free_table( dbResult );

    return ret;
}

static FINDEX_MGR_HANDLE file_index_mgr_instance( void )
{
    static FileIndexMgrObject *fimo = NULL;
    if ( NULL == fimo )
    {
        fimo = ( FileIndexMgrObject * )calloc( 1, sizeof( FileIndexMgrObject ) );
    }

    return fimo;
}

static int32 get_time_index_count( FINDEX_MGR_HANDLE handle )
{
    int8 **dbResult;
    int8* errmsg = NULL;
    int32 nRow, nColumn;
    int32 count = 0;
    int32 result;

    result = sqlite3_get_table( handle->timeDb, "select ID from TimeIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        // result store in dbResult[]
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            count = nRow;
        }
    }

    sqlite3_free_table( dbResult );

    return count;
}

static int32 get_time_index_first_id( FINDEX_MGR_HANDLE handle )
{
    int8 **dbResult;
    int32 nRow, nColumn;
    int32 id = 0;
    int32 result;
    int8* errmsg = NULL;

    result = sqlite3_get_table( handle->timeDb, "select ID from TimeIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        // result store in dbResult[]
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            id = atoi( dbResult[1] );
        }
    }

    sqlite3_free_table( dbResult );

    return id;
}

static int32 get_time_index_last_id( FINDEX_MGR_HANDLE handle )
{
    int8 **dbResult;
    int32 nRow, nColumn;
    int32 id = 0;
    int32 result;
    int8* errmsg = NULL;

    result = sqlite3_get_table( handle->timeDb, "select ID from TimeIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        // result store in dbResult[]
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            id = atoi( dbResult[nRow] );
        }
    }

    sqlite3_free_table( dbResult );

    return id;
}

static int32 get_event_index_count( FINDEX_MGR_HANDLE handle )
{
    int8 **dbResult;
    int32 nRow, nColumn;
    int32 count = 0;
    int32 result;
    int8* errmsg = NULL;

    result = sqlite3_get_table( handle->evtDb, "select ID from EventIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        // result store in dbResult[]
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            count = nRow;
        }
    }

    sqlite3_free_table( dbResult );

    return count;
}

static int32 get_event_index_last_id( FINDEX_MGR_HANDLE handle )
{
    int8 **dbResult;
    int32 nRow, nColumn;
    int32 id = 0;
    int32 result;
    int8* errmsg = NULL;

    result = sqlite3_get_table( handle->evtDb, "select ID from EventIndexTable", &dbResult, &nRow, &nColumn, &errmsg );
    if ( SQLITE_OK == result )
    {
        // result store in dbResult[]
        if ( ( nRow > 0 ) && ( nColumn > 0 ) )
        {
            id = atoi( dbResult[nRow] );
        }
    }
    sqlite3_free_table( dbResult );

    return id;
}

static boolean compare_time_interval( int8 *stime1, int8 *etime1, int8 * stime2, int8 * etime2 )
{
    // only while event index between time index.
    // format 2013-12-21 12:00:00
    boolean ret = FALSE;
    int32 ss1 = 0, se1 = 0, ss2 = 0, se2 = 0;

    ss1 = get_seconds( stime1 );
    se1 = get_seconds( etime1 );
    ss2 = get_seconds( stime2 );
    se2 = get_seconds( etime2 );

    // ----------------
    // -----------
    if ( ( ss1 <= ss2 ) && ( se1 >= se2 ) )
    {
        ret = TRUE;
    }

    return ret;
}

static boolean compare_time_interval2( int8 *stime1, int8 *etime1, int8 * stime2, int8 * etime2 )
{
    // only while event index between time index.
    // format 2013-12-21 12:00:00
    boolean ret = TRUE;
    int32 ss1 = 0, se1 = 0, ss2 = 0, se2 = 0;

    ss1 = get_seconds( stime1 );
    se1 = get_seconds( etime1 );
    ss2 = get_seconds( stime2 );
    se2 = get_seconds( etime2 );

    // ----------------
    //  -----------

    //   ----------------
    // -------------

    // ----------------
    //        -------------

    //    ----------------
    // --------------------------

    // ===============================================
    // ------
    //        --------------

    //          --------
    // -------

    if ( ( se1 <= ss2 ) || ( ss1 >= se2 ) )
    {
        ret = FALSE;
    }

    return ret;
}

static int32 get_seconds( int8 *strTime )
{
    int32 ret = 0;
    int32 year, mon, day, hour, min, sec;
    if ( 6 == sscanf( strTime, "%d-%d-%d %d:%d:%d", &year, &mon, &day, &hour, &min, &sec ) )
    {
        struct tm etm;
        memset( &etm, 0, sizeof( struct tm ) );
        etm.tm_year = year - 1900;
        etm.tm_mon  = mon - 1;
        etm.tm_mday = day;
        etm.tm_hour = hour;
        etm.tm_min  = min;
        etm.tm_sec  = sec;

        ret = ( int32 )mktime( &etm ); // time_t convert to int32.
    }

    return ret;
}

int32 file_index_mgr_unit_test( void )
{
    FINDEX_MGR_HANDLE handle;
    handle = file_index_mgr_create( "/mnt/cf/time_index.db", "/mnt/cf/event_index.db" );

    //int8  fileName[32] = { 0 };
    int8  stime[32] = { 0 };
    int8  etime[32] = { 0 };

    //int8 timeIndexDB[32] = { 0 };
    //int32 offset;
    int32 count;
    int8  fileName[32];
    int32 offset = 0;

    file_index_mgr_dump_time_index( handle );

    count = file_index_mgr_get_event_count( handle );
    printf("get event count %d \n", count );

    int32 start, end;
    if ( file_index_mgr_get_event_index_id_zone( handle, &start, &end ) )
    {
        int32 i;
        for( i=start; i<=end; i++ )
        {
            if ( file_index_mgr_get_event( handle, i, stime, etime ) )
            {
                file_index_mgr_get_file_according_to_event_id( handle, i, fileName, &offset );
                //printf("get event %s %s \n", stime, etime );
            }
        }
    }

#if 0
    file_index_mgr_get_last_time_index_param( handle, fileName, &offset );

    file_index_mgr_open_time_index_item( handle, "2013-12-21 12:00:01", "nhdz01.dat", offset );
    file_index_mgr_close_time_index_item( handle, "2013-12-21 12:00:02" );

    file_index_mgr_get_last_time_index_param( handle, fileName, &offset );

    file_index_mgr_open_time_index_item( handle, "2013-12-21 13:00:02", "nhdz02.dat", offset );
    file_index_mgr_close_time_index_item( handle, "2013-12-21 12:00:03" );

    file_index_mgr_get_last_time_index_param( handle, fileName, &offset );
    file_index_mgr_open_time_index_item( handle, "2013-12-21 14:00:03", "nhdz03.dat", offset );
    file_index_mgr_close_time_index_item( handle, "2013-12-21 12:00:04" );

    file_index_mgr_del_time_index_section_by_filename( handle, "nhdz02.dat" );
    count = get_time_index_count( handle );
#endif
#if 0
    //file_index_mgr_open_event( handle, "2013-12-21 12:00:00" );
    //file_index_mgr_close_event( handle, "2013-12-21 12:00:01" );

    //file_index_mgr_open_event( handle, "2013-12-21 12:00:01" );
    //file_index_mgr_close_event( handle, "2013-12-21 12:00:02" );

    file_index_mgr_open_event( handle, "2013-12-21 12:00:02", "nhdz0001.dat" );
    file_index_mgr_close_event( handle, "2013-12-21 12:00:03" );

    file_index_mgr_open_event( handle, "2013-12-21 12:00:03",  "nhdz0002.dat"  );
    file_index_mgr_close_event( handle, "2013-12-21 12:00:04" );

    delete_item_from_event_index_table( handle, "2013-12-21 12:00:02" );

    file_index_mgr_open_event( handle, "2013-12-21 12:00:04",  "nhdz0002.dat"  );
    file_index_mgr_close_event( handle, "2013-12-21 12:00:05" );

    count = file_index_mgr_get_event_count( handle );



    int32 timeCount = 0;
    int32 start, end;
    if ( file_index_mgr_get_event_index_id_zone( handle, &start, &end ) )
    {
        int32 i;
        for( i=start; i<=end; i++ )
        {
            if ( file_index_mgr_get_event( handle, i, stime, etime ) )
            {
                timeCount++;
            }
        }
    }


    count = file_index_mgr_get_event_count( handle );
#endif

    file_index_mgr_destroy( handle );

    return 0;
}
