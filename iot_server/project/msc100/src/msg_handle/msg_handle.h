#ifndef	__MSG_HANDLE_H
#define	__MSG_HANDLE_H

#ifdef __cplusplus
extern "C"
{
#endif

uint16 msg_handle_packet( int8 *out_buf, uint16 out_len, int8 from, int8 msg, int8 * in_buf, uint16 in_len );
uint16 msg_handle_unpacket( int8 *out_buf, uint16 out_len, int8 *from, int8 *msg, int8 *in_buf, uint16 in_len );

#ifdef __cplusplus
}
#endif

#endif //__MSG_HANDLE_H
