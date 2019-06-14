#ifndef __PARAM_XML_H
#define __PARAM_XML_H

#include <core/core.h>
#include <xml/xml_api.h>
#include "param_string.h"

#ifdef __cplusplus
extern "C" {
#endif

boolean xml_get_username( int8 *value );
boolean xml_get_password( int8 *value );

boolean xml_get_local_method( int8 *method );
boolean xml_get_local_ip( int8 *IP );
boolean xml_get_local_netmask( int8 *nm );
boolean xml_get_local_gateway( int8 *gw );

boolean xml_get_wlan_method( int8 *method );
boolean xml_get_wlan_ip( int8 *IP );
boolean xml_get_wlan_netmask( int8 *nm );
boolean xml_get_wlan_gateway( int8 *gw );

boolean xml_get_wlan_essid( int8 *value );
int32   xml_get_wlan_auth_mode( void );
boolean xml_get_wlan_password( int8 *value );

boolean xml_get_first_dns( int8 *value );
boolean xml_get_second_dns( int8 *value );
int32   xml_get_http_port( void );

// week_day from 0 - 6. 0 is sunday.
boolean xml_get_switch_time_plan( int32 week_day, int8 *value );
boolean xml_get_switch_time_plan_enable( void );

int32   xml_get_ntp_time_zone( void );
boolean xml_get_device_name( int8 *value );
boolean xml_get_id( int8 *value );
boolean xml_get_cpu_type( int8 *value );
boolean xml_get_soft_version( int8 *value );
boolean xml_get_wireless_work_mode( int8 *value );
boolean xml_get_wireless_ap_ip( int8 *value );
boolean xml_get_wireless_ap_essid( int8 *value );
boolean xml_get_wireless_ap_enc_mode( int8 *value ); // share or wpa
boolean xml_get_wireless_ap_password( int8 *value );
boolean xml_get_udp_server_ip( int32 index, int8 *value );
int32   xml_get_udp_server_port( void );
int32   xml_get_switch_on_off( int32 index );

// set param.
boolean xml_set_username( int8 *value );
boolean xml_set_password( int8 *value );
boolean xml_set_local_method( int8 *method );
boolean xml_set_local_ip( int8 *IP );
boolean xml_set_local_netmask( int8 *nm );
boolean xml_set_local_gateway( int8 *gw );
boolean xml_set_wlan_method( int8 *method );
boolean xml_set_wlan_ip( int8 *IP );
boolean xml_set_wlan_netmask( int8 *nm );
boolean xml_set_wlan_gateway( int8 *gw );

boolean xml_set_first_dns( int8 *value );
boolean xml_set_second_dns( int8 *value );
boolean xml_set_http_port( int32 value );

boolean xml_set_switch_on_off( int32 index, int32 value );

boolean xml_set_ntp_time_zone( int32 value );

boolean xml_set_device_name( int8 *value );
boolean xml_set_id( int8 *value );
boolean xml_set_cpu_type( int8 *value );
boolean xml_set_soft_version( int8 *value );

boolean xml_set_wireless_work_mode( int8 *value );

boolean xml_set_wlan_essid( int8 *value );
boolean xml_set_wlan_auth_mode( int32 value );
boolean xml_set_wlan_password( int8 *value );

boolean xml_set_wireless_ap_ip( int8 *value );
boolean xml_set_wireless_ap_essid( int8 *value );
boolean xml_set_wireless_ap_enc_mode( int8 *value ); // share or wpa
boolean xml_set_wireless_ap_password( int8 *value );

boolean xml_set_udp_server_ip( int32 index, int8 *value );
boolean xml_set_udp_server_port( int32 value );
boolean xml_set_switch_time_plan( int32 index, int8 *value );
boolean xml_set_switch_time_plan_enable( boolean enable );

#ifdef __cplusplus
}
#endif

#endif // __PARAM_XML_H
