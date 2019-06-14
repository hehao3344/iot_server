#include <stdio.h>
#include <string.h>

#include "prj_config.h"
#include "device/fs_mgr.h" // fix me
#include "param_xml.h"

typedef struct CameraParam
{
    int8  *key;
    int32 width;
    int32 height;
} CameraParam;

typedef struct ParamXmlObject
{
    XML_HANDLE hXml;
} ParamXmlObject;

static void  mtd_save_xml_callback( void *arg, const char *fileName )
{
    // 0 is Config_xxx.xml in mtd position.
#ifdef RT_PLATFORM
    debug_print("save %s to /dev/mtd3 index 2 \n", fileName);
    fs_mgr_save_config( 2, fileName );
#endif
}

static ParamXmlObject * param_xml_instance( void )
{
    static ParamXmlObject *pxo = NULL;
    if ( NULL == pxo )
    {
        pxo = ( ParamXmlObject *)calloc( 1, sizeof( ParamXmlObject ) );
        pxo->hXml = xml_create( CONFIG_FILE_NAME_XML );
        if ( NULL == pxo->hXml )
        {
            debug_print("xml file open failed %s \n", CONFIG_FILE_NAME_XML );
            free( pxo );
            pxo = NULL;
        }
        xml_set_callback( pxo->hXml, mtd_save_xml_callback, NULL );
    }

    return pxo;
}

boolean xml_get_username( int8 *value )
{
    int8 *username = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }

    username = xml_get_string( pxo->hXml,
                               LOGIN_SECTION,
                               LOGIN_USERNAME_KEY,
                               "admin" );


    if ( ( NULL != username ) && ( NULL != value ) )
    {
        strcpy( value, username );
    }

    return TRUE;
}

boolean xml_get_password( int8 *value )
{
    int8 *password = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }

    password = xml_get_string( pxo->hXml,
                               LOGIN_SECTION,
                               LOGIN_PASSWORD_KEY,
                               "admin" );


    if ( ( NULL != password ) && ( NULL != value ) )
    {
        strcpy( value, password );
    }

    return TRUE;
}

int32  xml_get_switch_on_off( int32 index )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }

    int8 key[16] = { 0 };
    sprintf( key, "%s%d", SWITCH_CH_KEY, index );
    return xml_get_int( pxo->hXml,
                        SWITCH_SECTION,
                        key,
                        0 );
}

boolean  xml_get_udp_send_ip( int32 index, int8 *value )
{
    int8 *name = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    if ( 0 == index )
    {
        name = xml_get_string( pxo->hXml,
                               UART_UDP_SECTION,
                               SEND_IP1_KEY,
                               "192.168.1.100" );
    }
    else
    {
        name = xml_get_string( pxo->hXml,
                               UART_UDP_SECTION,
                               SEND_IP2_KEY,
                               "192.168.1.101" );
    }

    if ( ( NULL != name ) && ( NULL != value ) )
    {
        strcpy( value, name );
    }

    return TRUE;
}

int32  xml_get_udp_send_port( int32 index )
{
    int32 ret = 0;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    if ( 0 == index )
    {
        ret = xml_get_int( pxo->hXml,
                           UART_UDP_SECTION,
                           SEND_PORT1_KEY,
                           8000 );
    }
    else
    {
        ret = xml_get_int( pxo->hXml,
                              UART_UDP_SECTION,
                              SEND_PORT2_KEY,
                              8001 );
    }

    return ret;
}

int32  xml_get_udp_send_enable( int32 index )
{
    int32 ret = 0;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    if ( 0 == index )
    {
        ret = xml_get_int( pxo->hXml,
                           UART_UDP_SECTION,
                           ENABLE1_KEY,
                           0 );
    }
    else
    {
        ret = xml_get_int( pxo->hXml,
                           UART_UDP_SECTION,
                           ENABLE2_KEY,
                           0 );
    }

    return ret;
}

int32 xml_get_ntp_time_zone( void )
{
    int32 ret = 0;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }

    ret = xml_get_int( pxo->hXml,
                       NTP_SECTION,
                       NTP_TIME_ZONE_KEY,
                       8 );

    return ret;
}

boolean xml_get_udp_server_ip( int32 index, int8 *value )
{
    int8 *name = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    if ( 0 == index )
    {
        name = xml_get_string( pxo->hXml,
                               UDP_SERVER_SECTION,
                               UDP_SERVER_IP1_KEY,
                               "192.168.1.100" );
    }
    else if ( 1 == index )
    {
        name = xml_get_string( pxo->hXml,
                               UDP_SERVER_SECTION,
                               UDP_SERVER_IP2_KEY,
                               "192.168.1.101" );
    }
    else
    {
        name = xml_get_string( pxo->hXml,
                               UDP_SERVER_SECTION,
                               UDP_SERVER_IP3_KEY,
                               "192.168.1.102" );
    }

    if ( ( NULL != name ) && ( NULL != value ) )
    {
        strcpy( value, name );
    }

    return TRUE;
}

int32 xml_get_udp_server_port( void )
{
    int32 ret = 0;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }

    ret = xml_get_int( pxo->hXml,
                       UDP_SERVER_SECTION,
                       UDP_SERVER_PORT_KEY,
                       6100 );

    return ret;
}
boolean  xml_get_device_name( int8 *value )
{
    int8 *name = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    name = xml_get_string( pxo->hXml,
                           DEVICE_SECTION,
                           NAME_KEY,
                           "device" );
    if ( ( NULL != name ) && ( NULL != value ) )
    {
        strcpy( value, name );
    }

    return TRUE;
}

boolean xml_get_id( int8 *value )
{
    int8 *name = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    name = xml_get_string( pxo->hXml,
                           DEVICE_SECTION,
                           ID_KEY,
                           "MCSZ0000000000000001" );
    if ( ( NULL != name ) && ( NULL != value ) )
    {
        strcpy( value, name );
    }

    return TRUE;
}

boolean xml_get_cpu_type( int8 *value )
{
    int8 *name = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    name = xml_get_string( pxo->hXml,
                           DEVICE_SECTION,
                           CPU_KEY,
                           "ARM" );
    if ( ( NULL != name ) && ( NULL != value ) )
    {
        strcpy( value, name );
    }

    return TRUE;
}

boolean xml_get_soft_version( int8 *value )
{
    int8 *name = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    name = xml_get_string( pxo->hXml,
                           DEVICE_SECTION,
                           SOFT_VERSION_KEY,
                           "20141007-1.01" );
    if ( ( NULL != name ) && ( NULL != value ) )
    {
        strcpy( value, name );
    }

    return TRUE;
}

boolean xml_get_wireless_ap_ip( int8 *value )
{
    int8 *ip = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    ip = xml_get_string( pxo->hXml,
                         WIRELESS_AP_SECTION,
                         WIRELESS_AP_IP_KEY,
                         "192.168.3.1" ); // or STA
    if ( ( NULL != ip ) && ( NULL != value ) )
    {
        strcpy( value, ip );
    }

    return TRUE;
}

boolean xml_get_wireless_ap_essid( int8 *value )
{
    int8 *name = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    name = xml_get_string( pxo->hXml,
                           WIRELESS_AP_SECTION,
                           WIRELESS_SSID_KEY,
                           "moc0001" ); // or STA
    if ( ( NULL != name ) && ( NULL != value ) )
    {
        strcpy( value, name );
    }

    return TRUE;
}

boolean xml_get_wireless_ap_enc_mode( int8 *value )
{
    int8 *mode = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    mode = xml_get_string( pxo->hXml,
                           WIRELESS_AP_SECTION,
                           WIRELESS_AP_ENC_KEY,
                           "share" );
    if ( ( NULL != mode ) && ( NULL != value ) )
    {
        strcpy( value, mode );
    }

    return TRUE;
}

boolean xml_get_wireless_ap_password( int8 *value )
{
    int8 *password = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    password = xml_get_string( pxo->hXml,
                               WIRELESS_AP_SECTION,
                               WIRELESS_AP_PASSWORD_KEY,
                               "11111111" );
    if ( ( NULL != password ) && ( NULL != value ) )
    {
        strcpy( value, password );
    }

    return TRUE;
}

boolean xml_get_wireless_work_mode( int8 *value )
{
    int8 *tmp = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    tmp = xml_get_string( pxo->hXml,
                          WIRELESS_WORK_PARAM_SECTION,
                          WIRELESS_MODE_KEY,
                          "sta" ); // or "STA"
    if ( ( NULL != tmp ) && ( NULL != value ) )
    {
        strcpy( value, tmp );
    }
    debug_print("xml get wifi mode %s \n", value );

    return TRUE;
}

boolean xml_get_local_method( int8 *method )
{
    int8 *value = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    value = xml_get_string( pxo->hXml,
                            LOCAL_ADDRESS_SECTION,
                            LOCAL_IP_METHOD,
                            "static" );
    if ( ( NULL != value ) && ( NULL != method ) )
    {
        strcpy( method, value );
    }
    debug_print("xml get local method %s \n", method );

    return TRUE;
}

boolean xml_get_local_ip( int8 *IP )
{
    int8 *value = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    value = xml_get_string( pxo->hXml,
                            LOCAL_ADDRESS_SECTION,
                            LOCAL_IP_KEY,
                            "192.168.1.125" );
    if ( ( NULL != value ) && ( NULL != IP ) )
    {
        strcpy( IP, value );
    }
    //debug_print("xml get local ip %s \n", IP );

    return TRUE;
}

boolean xml_get_local_netmask( int8 *nm )
{
    int8 *value = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    value = xml_get_string( pxo->hXml,
                            LOCAL_ADDRESS_SECTION,
                            LOCAL_NETMASK_KEY,
                            "255.255.255.0" );
    if ( ( NULL != value ) && ( NULL != nm ) )
    {
        strcpy( nm, value );
    }
    //debug_print("xml get local netmask %s \n", nm );

    return TRUE;
}

boolean xml_get_local_gateway( int8 *gw )
{
    int8 *value = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    value = xml_get_string( pxo->hXml,
                            LOCAL_ADDRESS_SECTION,
                            LOCAL_GATEWAY_KEY,
                            "192.168.1.1" );
    if ( ( NULL != value ) && ( NULL != gw ) )
    {
        strcpy( gw, value );
    }
    //debug_print("xml get local gateway %s \n", gw );

    return TRUE;
}

boolean xml_get_wlan_method( int8 *method )
{
    int8 *value = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    value = xml_get_string( pxo->hXml,
                            LOCAL_ADDRESS_SECTION,
                            WLAN_IP_METHOD,
                            "static" );
    if ( ( NULL != value ) && ( NULL != method ) )
    {
        strcpy( method, value );
    }
    debug_print("xml get local wlan method %s \n", method );

    return TRUE;
}

boolean xml_get_wlan_ip( int8 *IP )
{
    int8 *value = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    value = xml_get_string( pxo->hXml,
                            LOCAL_ADDRESS_SECTION,
                            WLAN_IP_KEY,
                            "192.168.1.168" );
    if ( ( NULL != value ) && ( NULL != IP ) )
    {
        strcpy( IP, value );
    }
    debug_print("xml get local wlan ip %s \n", IP );

    return TRUE;
}

boolean xml_get_wlan_netmask( int8 *nm )
{
    int8 *value = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    value = xml_get_string( pxo->hXml,
                            LOCAL_ADDRESS_SECTION,
                            WLAN_NETMASK_KEY,
                            "255.255.255.0" );
    if ( ( NULL != value ) && ( NULL != nm ) )
    {
        strcpy( nm, value );
    }
    debug_print("xml get local wlan netmask %s \n", nm );

    return TRUE;
}

boolean xml_get_wlan_gateway( int8 *gw )
{
    int8 *value = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    value = xml_get_string( pxo->hXml,
                            LOCAL_ADDRESS_SECTION,
                            WLAN_GATEWAY_KEY,
                            "192.168.1.1" );
    if ( ( NULL != value ) && ( NULL != gw ) )
    {
        strcpy( gw, value );
    }
    debug_print("xml get local wlan gateway %s \n", gw );

    return TRUE;
}

boolean xml_get_wlan_essid( int8 *value )
{
    int8 *essid = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    essid = xml_get_string( pxo->hXml,
                            WLAN_AUTH_SECTION,
                            WLAN_ESSID_KEY,
                            "wlan-essid" );
    if ( ( NULL != value ) && ( NULL != essid ) )
    {
        strcpy( value, essid );
    }
    // debug_print("xml get wlan essid %s \n", value );

    return TRUE;
}

int32   xml_get_wlan_auth_mode( void )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return 0;
    }

    return xml_get_int( pxo->hXml,
                        WLAN_AUTH_SECTION,
                        WLAN_MODE_KEY,
                        1 ); // 1 share, 2, wep, 3, wpa...
}

boolean xml_get_wlan_password( int8 *value )
{
    int8 *pwd = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    pwd = xml_get_string( pxo->hXml,
                          WLAN_AUTH_SECTION,
                          WLAN_PASSWORD_KEY,
                          "wlan-pass" );
    if ( ( NULL != value ) && ( NULL != pwd ) )
    {
        strcpy( value, pwd );
    }
    // debug_print("xml get wlan password %s \n", value );

    return TRUE;
}

boolean xml_get_first_dns( int8 *value )
{
    int8 *dns = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    dns = xml_get_string( pxo->hXml,
                           LOCAL_ADDRESS_SECTION,
                            FIRST_DNS,
                            "192.168.1.1" );
    if ( ( NULL != dns ) && ( NULL != value ) )
    {
        strcpy( value, dns );
    }
    debug_print("xml get local first dns %s \n", value );

    return TRUE;
}

boolean xml_get_second_dns( int8 *value )
{
    int8 *dns = NULL;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    dns = xml_get_string( pxo->hXml,
                           LOCAL_ADDRESS_SECTION,
                            SECOND_DNS,
                            "192.168.1.3" );
    if ( ( NULL != dns ) && ( NULL != value ) )
    {
        strcpy( value, dns );
    }
    debug_print("xml get local second dns %s \n", value );

    return TRUE;
}

int32 xml_get_http_port( void )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return 0;
    }

    return xml_get_int( pxo->hXml,
                        LOCAL_ADDRESS_SECTION,
                        HTTP_PORT,
                        80 );
}

////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// xml set param
/////////////////////////////////////////////////////////////
boolean xml_set_username( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    LOGIN_SECTION,
                    LOGIN_USERNAME_KEY,
                    value );

    return TRUE;
}

boolean xml_set_password( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    LOGIN_SECTION,
                    LOGIN_PASSWORD_KEY,
                    value );

    return TRUE;
}

boolean xml_set_local_method( int8 *method )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    LOCAL_ADDRESS_SECTION,
                    LOCAL_IP_METHOD,
                    method );

    return TRUE;
}

boolean xml_set_local_ip( int8 *IP )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    LOCAL_ADDRESS_SECTION,
                    LOCAL_IP_KEY,
                    IP );

    return TRUE;
}

boolean xml_set_local_netmask( int8 *nm )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    LOCAL_ADDRESS_SECTION,
                    LOCAL_NETMASK_KEY,
                    nm );

    return TRUE;
}

boolean xml_set_local_gateway( int8 *gw )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    LOCAL_ADDRESS_SECTION,
                    LOCAL_GATEWAY_KEY,
                    gw );

    return TRUE;
}

boolean xml_set_wlan_method( int8 *method )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    LOCAL_ADDRESS_SECTION,
                    WLAN_IP_METHOD,
                    method );

    return TRUE;
}

boolean xml_set_wlan_ip( int8 *IP )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    LOCAL_ADDRESS_SECTION,
                    WLAN_IP_KEY,
                    IP );

    return TRUE;
}

boolean xml_set_wlan_netmask( int8 *nm )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    LOCAL_ADDRESS_SECTION,
                    WLAN_NETMASK_KEY,
                    nm );

    return TRUE;
}

boolean xml_set_wlan_gateway( int8 *gw )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    LOCAL_ADDRESS_SECTION,
                    WLAN_GATEWAY_KEY,
                    gw );

    return TRUE;
}

boolean xml_set_wlan_essid( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    WLAN_AUTH_SECTION,
                    WLAN_ESSID_KEY,
                    value );

    return TRUE;
}

boolean xml_set_wlan_auth_mode( int32 value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_int( pxo->hXml,
                 WLAN_AUTH_SECTION,
                 WLAN_MODE_KEY,
                 value );

    return TRUE;
}

boolean xml_set_wlan_password( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    WLAN_AUTH_SECTION,
                    WLAN_PASSWORD_KEY,
                    value );

    return TRUE;
}

boolean xml_set_first_dns( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    LOCAL_ADDRESS_SECTION,
                    FIRST_DNS,
                    value );

    return TRUE;
}

boolean xml_set_second_dns( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    LOCAL_ADDRESS_SECTION,
                    SECOND_DNS,
                    value );

    return TRUE;
}

boolean xml_set_http_port( int32 value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_int( pxo->hXml,
                    LOCAL_ADDRESS_SECTION,
                    HTTP_PORT,
                    value );

    return TRUE;
}

boolean xml_set_switch_on_off( int32 index, int32 value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }

    int8 key[16] = { 0 };
    sprintf( key, "%s%d", SWITCH_CH_KEY, index );

    xml_set_int( pxo->hXml,
                 SWITCH_SECTION,
                 key,
                 value );

    return TRUE;
}

// the min length of value is 64.
boolean xml_get_switch_time_plan( int32 week_day, int8 *value )
{
    int8 *plan = NULL;
    int8 weekday[16] = { 0 };
    sprintf( weekday, "%s%d", WEEK_DAY_KEY, week_day );

    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return 0;
    }

    plan = xml_get_string( pxo->hXml,
                           REC_PLAN_SECTION,
                           weekday,
                           "0:0:0-0:0:0,0:0:0-0:0:0,0:0:0-0:0:0" );
    if ( ( NULL != plan ) && ( NULL != value ) )
    {
        strcpy( value, plan );
    }

    return TRUE;
}

boolean xml_get_switch_time_plan_enable( void )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return 0;
    }

    return ( boolean )xml_get_int( pxo->hXml,
                                   REC_PLAN_SECTION,
                                   REC_ENABLE_KEY,
                                   0 );
}

boolean xml_set_ntp_time_zone( int32 value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_int( pxo->hXml,
                 NTP_SECTION,
                 NTP_TIME_ZONE_KEY,
                 value );

    return TRUE;
}

boolean xml_set_device_name( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                 DEVICE_SECTION,
                 NAME_KEY,
                 value );

    return TRUE;
}

boolean xml_set_id( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    DEVICE_SECTION,
                    ID_KEY,
                    value );
    return TRUE;
}

boolean xml_set_cpu_type( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    DEVICE_SECTION,
                    CPU_KEY,
                    value );

    return TRUE;
}


boolean xml_set_soft_version( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    DEVICE_SECTION,
                    SOFT_VERSION_KEY,
                    value );

    return TRUE;
}

boolean xml_set_wireless_ap_ip( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    WIRELESS_AP_SECTION,
                    WIRELESS_AP_IP_KEY,
                    value );

    return TRUE;
}

boolean xml_set_wireless_ap_essid( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    WIRELESS_AP_SECTION,
                    WIRELESS_SSID_KEY,
                    value );

    return TRUE;
}

boolean xml_set_wireless_ap_enc_mode( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    WIRELESS_AP_SECTION,
                    WIRELESS_AP_ENC_KEY,
                    value );

    return TRUE;
}

boolean xml_set_wireless_ap_password( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    WIRELESS_AP_SECTION,
                    WIRELESS_AP_PASSWORD_KEY,
                    value );

    return TRUE;
}

boolean xml_set_wireless_work_mode( int8 *value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    WIRELESS_WORK_PARAM_SECTION,
                    WIRELESS_MODE_KEY,
                    value );

    return TRUE;
}

boolean xml_set_udp_server_ip( int32 index, int8 *value )
{
    boolean ret = FALSE;
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    int8 key[16];
    memset( key, 0, sizeof( key ) );

    if ( index < 3 )
    {
        sprintf( key, "IP%d", ( index + 1 ) );

        xml_set_string( pxo->hXml,
                        UDP_SERVER_SECTION,
                        key,
                        value );
        ret = TRUE;
    }

    return ret;
}

boolean xml_set_udp_server_port( int32 value )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }

    xml_set_int( pxo->hXml,
                 UDP_SERVER_SECTION,
                 UDP_SERVER_PORT_KEY,
                 value );
    return TRUE;
}

boolean xml_set_switch_time_plan( int32 index, int8 *value )
{
    int8 weekday[16] = { 0 };
    sprintf( weekday, "%s%d", WEEK_DAY_KEY, index );

    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_string( pxo->hXml,
                    REC_PLAN_SECTION,
                    weekday,
                    value );

    return TRUE;
}

boolean xml_set_switch_time_plan_enable( boolean enable )
{
    ParamXmlObject *pxo = param_xml_instance();
    if ( NULL == pxo )
    {
        return FALSE;
    }
    xml_set_int( pxo->hXml,
                 REC_PLAN_SECTION,
                 REC_ENABLE_KEY,
                 enable );

    return TRUE;
}
