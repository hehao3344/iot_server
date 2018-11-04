#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "core/core.h"
#include "msg_handle/msg_handle.h"

#include "db/group_manage.h"

int main(int argc, char** argv)
{
    group_mgr_unit_test();

    return 0;

#ifdef LINUX
    signal( SIGPIPE, SIG_IGN );
#endif

#if 0
    int i;
    int8 out_buf[32];
    int8 in_buf[8] = { 0, 1, 2, 3 };
    uint16 ret = msg_handle_packet( out_buf, sizeof( out_buf ), 0x03, 0x10, in_buf, 4 );

    if ( ret > 0 )
    {
        printf("packet success %d \n", ret );
    }

    int8 from, msg;
    memset( in_buf, 0, sizeof( in_buf ) );
    from = msg = 0;
    ret = msg_handle_unpacket( in_buf, sizeof( in_buf ), &from, &msg, out_buf, 13 );
    if ( ret > 0 )
    {
        printf("unpacket success %d \n", ret );
        for( i=0; i<ret; i++ )
        {
            printf("0x%x ", in_buf[i]);
        }

    }
#endif
    //uint16 msg_handle_unpacket( int8 *out_buf, uint16 out_len, uint8 *from, uint8 *msg, int8 *in_buf, uint16 in_len );
#if 0
    uint32 count = 1;
    TCP_SERVER_HANDLE htcp_server = tcp_server_create();
    while( 1 )
    {
        os_sleep_sec( 2 );
        if ( 0 == ( count++%1800 ) )
        {
            printf("system run for %d hours. \n", count/1800 );
        }
    }
    tcp_server_destroy( htcp_server );
#endif

    return 0;
}
