#ifndef __PARAM_INI_H
#define __PARAM_INI_H

#include "param_string.h"
#include <base/ini_file.h>

#ifdef __cplusplus
extern "C" {
#endif

boolean ini_get_capture_res( int32 *width, int32 *height );
int32   ini_get_encoder( int32 index );
boolean ini_get_enc_wxh( int32 index, int32 *width, int32 *height );
int32   ini_get_framerate( int32 index );
int32   ini_get_enc_quality( int32 index );
boolean ini_get_enc_mode( int32 index, int8 *value );
int32   ini_get_bitrate( int32 index );
int32   ini_get_gop( int32 index );
void    ini_get_sender_ip( const int32 index, int8 * buffer );
int32   ini_get_sender_port( const int32 index );
int32   ini_get_xor( const int32 index );
int32   ini_get_osd_enable( const int32 index );

boolean ini_get_local_method( int8 *method );
boolean ini_get_local_ip( int8 *IP );
boolean ini_get_local_netmask( int8 *nm );
boolean ini_get_local_gateway( int8 *gw );

boolean ini_get_wlan_method( int8 *method );
boolean ini_get_wlan_ip( int8 *IP );
boolean ini_get_wlan_netmask( int8 *nm );
boolean ini_get_wlan_gateway( int8 *gw );

boolean ini_get_first_dns( int8 *value );
boolean ini_get_second_dns( int8 *value );
int32   ini_get_http_port( void );

boolean ini_get_ftp_server( int8 *value );
boolean ini_get_ftp_username( int8 *value );
boolean ini_get_ftp_password( int8 *value );
boolean ini_get_ftp_directory( int8 *value );
boolean ini_get_ftp_enable( void );

int32   ini_get_record_plan( int32 index );
boolean ini_get_record_enable( void );
int32   ini_get_record_method( void );

boolean ini_get_ddns_server( int8 *value );
boolean ini_get_ddns_username( int8 *value );
boolean ini_get_ddns_password( int8 *value );
boolean ini_get_ddns_enable( void );

boolean ini_get_smtp_server( int8 *value );
boolean ini_get_smtp_server2( int8 *value );
boolean ini_get_smtp_username( int8 *value );
boolean ini_get_smtp_password( int8 *value );
boolean ini_get_smtp_from_address( int8 *value );
boolean ini_get_smtp_to_address( int8 *value );
boolean ini_get_smtp_to_address2( int8 *value );
boolean ini_get_smtp_enable( void );

int32   ini_get_upnp_internal_port( void );
int32   ini_get_upnp_external_port( void );
boolean ini_get_upnp_enable( void );

int32   ini_get_ntp_time_zone( void );
boolean ini_get_device_name( int8 *value );

// set param.
boolean ini_set_capture_res( int8 *value );
boolean ini_set_enc_res( int32 index, int8 *value );
boolean ini_set_encoder( int32 index, int8 *value );
boolean ini_set_framerate( int32 index, int32 value );
boolean ini_set_enc_mode( int32 index, int32 value );
boolean ini_set_enc_quality( int32 index, int32 value );
boolean ini_set_bitrate( int32 index, int32 value );
boolean ini_set_gop( int32 index, int32 value );
boolean ini_set_osd_enable( int32 index, int32 value );
boolean ini_set_xor( int32 index, int32 value );
//boolean ini_set_get_rec_plan( int32 index, int8 * start, int8 * end );
//boolean ini_set_set_rec_plan( int8 * start, int8 * end );
boolean ini_set_clear_all_rec_plan( void );
boolean ini_set_audio_switch( int32 index, int32 value );

boolean ini_set_local_method( int8 *method );
boolean ini_set_local_ip( int8 *IP );
boolean ini_set_local_netmask( int8 *nm );
boolean ini_set_local_gateway( int8 *gw );
boolean ini_set_first_dns( int8 *value );
boolean ini_set_second_dns( int8 *value );
boolean ini_set_http_port( int32 value );

boolean ini_set_ftp_server( int8 *value );
boolean ini_set_ftp_username( int8 *value );
boolean ini_set_ftp_password( int8 *value );
boolean ini_set_ftp_directory( int8 *value );
boolean ini_set_ftp_enable( boolean enable );
boolean ini_set_record_plan( int32 index, int32 value );
boolean ini_set_record_enable( boolean enable );
boolean ini_set_record_method( int32 value );

boolean ini_set_ddns_server( int8 *value );
boolean ini_set_ddns_username( int8 *value );
boolean ini_set_ddns_password( int8 *value );
boolean ini_set_ddns_enable( boolean enable );

boolean ini_set_smtp_server( int8 *value );
boolean ini_set_smtp_server2( int8 *value );
boolean ini_set_smtp_username( int8 *value );
boolean ini_set_smtp_password( int8 *value );
boolean ini_set_smtp_from_address( int8 *value );
boolean ini_set_smtp_to_address( int8 *value );
boolean ini_set_smtp_to_address2( int8 *value );
boolean ini_set_smtp_enable( boolean enable );

boolean ini_set_upnp_internal_port( int32 value );
boolean ini_set_upnp_external_port( int32 value );
boolean ini_set_upnp_enable( boolean enable );

boolean ini_set_ntp_time_zone( int32 value );

boolean ini_set_device_name( int8 *value );

#ifdef __cplusplus
}
#endif

#endif // __PARAM_INI_H
