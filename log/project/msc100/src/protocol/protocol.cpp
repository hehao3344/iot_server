#include <stdlib.h>
#include <string.h>

#ifdef LINUX
#include <arpa/inet.h>
#endif

#include "core/types.h"
#include "msg_define.h"
#include "msg_handle.h"

uint16 msg_handle_packet( uint8 *out_buf, uint16 out_len, uint8 from, uint8 msg, uint8 * in_buf, uint16 in_len )
{
    if ( out_len < ( in_len + 9 ) )  // 9 = 2 + 2 + 1 + 1 + 1 + 2
    {
        printf("msg_handle_packet error \n");
        return 0;
    }

    uint16 i;
    uint16 ret = 0;
    uint8  check_sum = 0;

    FrameHead frame_head;
    frame_head.head   = 0xA55A;
    frame_head.length = 9 + in_len;     // 9 = 2 + 2 + 1 + 1 + 1 + 2
    frame_head.from   = from;
    frame_head.msg    = msg;

    memcpy( out_buf, &frame_head, sizeof( FrameHead ) );
    memcpy( out_buf + sizeof( FrameHead ), in_buf, in_len );

    for ( i=0; i<(in_len+4); i++ )  // 2 + 1 + 1 + N
    {
        check_sum = check_sum + out_buf[2+i];
    }

    out_buf[in_len+6] = check_sum;  // 2 + 2 + 1 + 1 + in_len
    out_buf[in_len+7] = 0x0D;
    out_buf[in_len+8] = 0x0A;

    ret = in_len + 9;

    return ret;
}

uint16 msg_handle_unpacket( uint8 *out_buf, uint16 out_len, uint8 *from, uint8 *msg, uint8 *in_buf, uint16 in_len )
{
    if ( ( in_len < 9 ) || ( out_len < ( in_len - 9 ) ) )
    {
        printf("msg_handle_unpacket error \n");
        return 0;
    }

    uint16 i;
    uint16 ret = 0;
    uint8  check_sum = 0;

    FrameHead frame_head;
    memcpy( &frame_head, in_buf, sizeof( FrameHead ) );

    if ( 0xA55A != frame_head.head )
    {
        printf("msg_handle_unpacket head error 0x%x \n", frame_head.head );
        return 0;
    }

    for ( i=0; i<(in_len-5); i++ )  // 2 + 2 + 1 + 1 + N + 1 + 2 => 2 + 1 + 1 + N
    {
        check_sum = check_sum + in_buf[2+i];
    }
    if ( check_sum != in_buf[in_len-3] )
    {
        printf("msg_handle_unpacket check sun error 0x%x != 0x%x \n", check_sum, in_buf[in_len-3] );
        return 0;
    }

    *from = frame_head.from;
    *msg  = frame_head.msg;

    ret = in_len - 9;
    memcpy( out_buf, in_buf + 6, ret );

    return ret;
}
