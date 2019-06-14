#include <stdio.h>
#include <string.h>

#include "prj_config.h"
#include "param_ini.h"

typedef struct CameraParam
{
    int8  *key;
    int32 width;
    int32 height;
} CameraParam;

static const CameraParam CameraParamTable[] =
{
    { RES_1920X1080_STRING, 1920, 1080, },
    { RES_1280X720_STRING,  1280, 720,  },
    { RES_640X480_STRING,   640,  480,  },
    { RES_720X576_STRING,   720,  576,  },
    { RES_704X576_STRING,   704,  576,  },
    { RES_704X288_STRING,   704,  288,  },
    { RES_352X288_STRING,   352,  288   },
    { RES_320X240_STRING,   320,  240   },
    { RES_176X144_STRING,   176,  144   },
};

boolean ini_get_capture_res( int32 *width, int32 *height )
{
    int32 i;
    boolean ret = FALSE;
    int8 resolution[16] = { 0 };

    GetPrivateProfileString( CH1_MAIN_CAPTURE_PARAM_SECTION,
                             CH1_MAIN_CAPTURE_RESOLUTION_KEY,
				             "1920X1080",
				             resolution,
				             sizeof( resolution ),
				             CONFIG_FILE_NAME );

    for ( i=0; i< ( sizeof( CameraParamTable )/sizeof(CameraParamTable[0]) ); i++ )
    {
        if ( 0 == strcmp( CameraParamTable[i].key, resolution ) )
        {
            *width = CameraParamTable[i].width;
            *height = CameraParamTable[i].height;
            ret = TRUE;
            break;
        }
    }

    return ret;
}

// return value: 1 - h.264
// return value: 2 - mjpeg
int32 ini_get_encoder( int32 index )
{
    int32 type = 1;
    int8 encoder[16] = { 0 };
    if ( 0 == index )
    {
        GetPrivateProfileString( CH1_MAIN_VIDEO_PARAM_SECTION,
                                 CH1_MAIN_VIDEO_ENCODER_KEY,
    				             "h264",
    				             encoder,
    				             sizeof( encoder ),
    				             CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        GetPrivateProfileString( CH1_SUB1_VIDEO_PARAM_SECTION,
                                 CH1_SUB1_VIDEO_ENCODER_KEY,
    				             "h264",
    				             encoder,
    				             sizeof( encoder ),
    				             CONFIG_FILE_NAME );
    }
    else if ( 2 == index )
    {
        GetPrivateProfileString( CH2_MAIN_VIDEO_PARAM_SECTION,
                                 CH2_MAIN_VIDEO_ENCODER_KEY,
    				             "h264",
    				             encoder,
    				             sizeof( encoder ),
    				             CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        GetPrivateProfileString( CH2_SUB1_VIDEO_PARAM_SECTION,
                                 CH2_SUB1_VIDEO_ENCODER_KEY,
    				             "h264",
    				             encoder,
    				             sizeof( encoder ),
    				             CONFIG_FILE_NAME );
    }

    if ( 0 == strcmp( encoder, "h264" ) )
    {
        type = 1;
    }
    else if ( 0 == strcmp( encoder, "mjpeg" ) )
    {
        type = 2;
    }

    return type;
}

boolean ini_get_enc_wxh( int32 index, int32 *width, int32 *height )
{
    int32 i;
    int32 ret = FALSE;
    int8 resolution[16] = { 0 };
    if ( 0 == index )
    {
        GetPrivateProfileString( CH1_MAIN_VIDEO_PARAM_SECTION,
                                 CH1_MAIN_VIDEO_RESOLUTION_KEY,
    				             "640X480",
    				             resolution,
    				             sizeof( resolution ),
    				             CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        GetPrivateProfileString( CH1_SUB1_VIDEO_PARAM_SECTION,
                                 CH1_SUB1_VIDEO_RESOLUTION_KEY,
    				             "640X480",
    				             resolution,
    				             sizeof( resolution ),
    				             CONFIG_FILE_NAME );
    }
    else if ( 2 == index )
    {
        GetPrivateProfileString( CH2_MAIN_VIDEO_PARAM_SECTION,
                                 CH2_MAIN_VIDEO_RESOLUTION_KEY,
    				             "640X480",
    				             resolution,
    				             sizeof( resolution ),
    				             CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        GetPrivateProfileString( CH2_SUB1_VIDEO_PARAM_SECTION,
                                 CH2_SUB1_VIDEO_RESOLUTION_KEY,
    				             "640X480",
    				             resolution,
    				             sizeof( resolution ),
    				             CONFIG_FILE_NAME );
    }
    debug_print( "Get resolution is %s of index %d \n", resolution, index);
    for ( i=0; i< ( sizeof( CameraParamTable )/sizeof(CameraParamTable[0]) ); i++ )
    {
        if ( 0 == strcmp( CameraParamTable[i].key, resolution ) )
        {
            *width = CameraParamTable[i].width;
            *height = CameraParamTable[i].height;
            ret = TRUE;
            break;
        }
    }

    return ret;
}

int32 ini_get_framerate( int32 index )
{
    int32 gop = 0;
    if ( 0 == index )
    {
        gop = GetPrivateProfileInt( CH1_MAIN_VIDEO_PARAM_SECTION,
                                    CH1_MAIN_VIDEO_FRAME_RATE_KEY,
            		                25,
            			            CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        gop = GetPrivateProfileInt( CH1_SUB1_VIDEO_PARAM_SECTION,
                                    CH1_SUB1_VIDEO_FRAME_RATE_KEY,
            		                25,
            			            CONFIG_FILE_NAME );
    }
    else if ( 2 == index )
    {
        gop = GetPrivateProfileInt( CH2_MAIN_VIDEO_PARAM_SECTION,
                                    CH2_MAIN_VIDEO_FRAME_RATE_KEY,
            		                25,
            			            CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        gop = GetPrivateProfileInt( CH2_SUB1_VIDEO_PARAM_SECTION,
                                    CH2_SUB1_VIDEO_FRAME_RATE_KEY,
            		                25,
            			            CONFIG_FILE_NAME );
    }

    return gop;
}

boolean ini_get_enc_mode( int32 index, int8 *value )
{
    if ( 0 == index )
    {
        GetPrivateProfileString( CH1_MAIN_VIDEO_PARAM_SECTION,
                                 CH1_MAIN_VIDEO_ENC_MODE_KEY,
    				             "cbr",
    				             value,
    				             16,
    				             CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        GetPrivateProfileString( CH1_SUB1_VIDEO_PARAM_SECTION,
                                 CH1_SUB1_VIDEO_ENC_MODE_KEY,
    				             "cbr",
    				             value,
    				             16,
    				             CONFIG_FILE_NAME );
    }
    else if ( 2 == index )
    {
        GetPrivateProfileString( CH2_MAIN_VIDEO_PARAM_SECTION,
                                 CH2_MAIN_VIDEO_ENC_MODE_KEY,
    				             "cbr",
    				             value,
    				             16,
    				             CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        GetPrivateProfileString( CH2_SUB1_VIDEO_PARAM_SECTION,
                                 CH2_SUB1_VIDEO_ENC_MODE_KEY,
    				             "cbr",
    				             value,
    				             16,
    				             CONFIG_FILE_NAME );
    }

    return TRUE;
}

int32   ini_get_enc_quality( int32 index )
{
    int32 value = 0;
    if ( 0 == index )
    {
        value = GetPrivateProfileInt( CH1_MAIN_VIDEO_PARAM_SECTION,
                                      CH1_MAIN_VIDEO_FRAME_RATE_KEY,
            		                  1,
            			              CH1_MAIN_VIDEO_QUALITY_KEY );
    }
    else if ( 1 == index )
    {
        value = GetPrivateProfileInt( CH1_SUB1_VIDEO_PARAM_SECTION,
                                      CH1_SUB1_VIDEO_FRAME_RATE_KEY,
            		                  1,
            			              CH1_SUB1_VIDEO_QUALITY_KEY );
    }
    else if ( 2 == index )
    {
        value = GetPrivateProfileInt( CH2_MAIN_VIDEO_PARAM_SECTION,
                                      CH2_MAIN_VIDEO_QUALITY_KEY,
            		                  1,
            			              CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        value = GetPrivateProfileInt( CH2_SUB1_VIDEO_PARAM_SECTION,
                                      CH2_SUB1_VIDEO_QUALITY_KEY,
            		                  1,
            			              CONFIG_FILE_NAME );
    }

    return value;
}

int32 ini_get_bitrate( int32 index )
{
    int32 value = 0;
    if ( 0 == index )
    {
        value = GetPrivateProfileInt( CH1_MAIN_VIDEO_PARAM_SECTION,
                                      CH1_MAIN_VIDEO_BITRATE_KEY,
            		                  2000000,
            			              CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        value = GetPrivateProfileInt( CH1_SUB1_VIDEO_PARAM_SECTION,
                                      CH1_SUB1_VIDEO_BITRATE_KEY,
            		                  1000000,
            			              CONFIG_FILE_NAME );
    }
    else if ( 2 == index )
    {
        value = GetPrivateProfileInt( CH2_MAIN_VIDEO_PARAM_SECTION,
                                      CH2_MAIN_VIDEO_BITRATE_KEY,
            		                  2000000,
            			              CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        value = GetPrivateProfileInt( CH2_SUB1_VIDEO_PARAM_SECTION,
                                      CH2_SUB1_VIDEO_BITRATE_KEY,
            		                  1000000,
            			              CONFIG_FILE_NAME );
    }

    return value;
}

int32 ini_get_gop( int32 index )
{
    int32 gop = 0;
    if ( 0 == index )
    {
        gop = GetPrivateProfileInt( CH1_MAIN_VIDEO_PARAM_SECTION,
                                    CH1_MAIN_VIDEO_GOP_KEY,
            		                25,
            			            CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        gop = GetPrivateProfileInt( CH1_SUB1_VIDEO_PARAM_SECTION,
                                    CH1_SUB1_VIDEO_GOP_KEY,
            		                25,
            			            CONFIG_FILE_NAME );
    }
    else if ( 2 == index )
    {
        gop = GetPrivateProfileInt( CH2_MAIN_VIDEO_PARAM_SECTION,
                                    CH2_MAIN_VIDEO_GOP_KEY,
            		                25,
            			            CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        gop = GetPrivateProfileInt( CH2_SUB1_VIDEO_PARAM_SECTION,
                                    CH2_SUB1_VIDEO_GOP_KEY,
            		                25,
            			            CONFIG_FILE_NAME );
    }

    return gop;
}

////////////////////////////////////////////////////////////////////
void ini_get_sender_ip( const int32 index, int8 * buffer )
{
    switch ( index )
    {
        case 0:
        {
            GetPrivateProfileString( CH1_RTP_SENDER_SECTION,
                                     CH1_SENDER_IP_KEY,
        				             "192.168.1.217",
        				             buffer,
        				             16,
        				             CONFIG_FILE_NAME );
            break;
        }
        case 1:
        {
            GetPrivateProfileString( CH2_RTP_SENDER_SECTION,
                                     CH2_SENDER_IP_KEY,
        				             "192.168.1.217",
        				             buffer,
        				             16,
        				             CONFIG_FILE_NAME );
            break;
        }
        case 2:
        {
            GetPrivateProfileString( CH3_RTP_SENDER_SECTION,
                                     CH3_SENDER_IP_KEY,
        				             "192.168.1.217",
        				             buffer,
        				             16,
        				             CONFIG_FILE_NAME );
            break;
        }
        case 3:
        {
            GetPrivateProfileString( CH4_RTP_SENDER_SECTION,
                                     CH4_SENDER_IP_KEY,
        				             "192.168.1.217",
        				             buffer,
        				             16,
        				             CONFIG_FILE_NAME );
            break;
        }
        default:
            debug_print( "invalid index %d \n", index);
            break;
    }
}

int32 ini_get_sender_port( const int32 index )
{
    int32 value = 8080;
    switch ( index )
    {
        case 0:
        {
            value = GetPrivateProfileInt( CH1_RTP_SENDER_SECTION,
                                         CH1_SENDER_PORT_KEY,
            		                     8080,
            			                 CONFIG_FILE_NAME );
            break;
        }
        case 1:
        {
            value = GetPrivateProfileInt( CH2_RTP_SENDER_SECTION,
                                         CH2_SENDER_PORT_KEY,
            		                     8080,
            			                 CONFIG_FILE_NAME );
            break;
        }
        case 2:
        {
            value = GetPrivateProfileInt( CH3_RTP_SENDER_SECTION,
                                         CH3_SENDER_PORT_KEY,
            		                     8080,
            			                 CONFIG_FILE_NAME );
            break;
        }
        case 3:
        {
            value = GetPrivateProfileInt( CH4_RTP_SENDER_SECTION,
                                         CH4_SENDER_PORT_KEY,
            		                     8080,
            			                 CONFIG_FILE_NAME );
            break;
        }
        default:
            debug_print( "invalid index %d \n", index);
            break;
    }

    return value;
}

int32 ini_get_xor( const int32 index )
{
    int32 value = 0;
    switch ( index )
    {
        case 0:
        {
            value = GetPrivateProfileInt( CH1_RTP_SENDER_SECTION,
                                          CH1_SENDER_USE_XOR,
                                          0,
                                          CONFIG_FILE_NAME );
            break;
        }
        case 1:
        {
            value = GetPrivateProfileInt( CH2_RTP_SENDER_SECTION,
                                          CH2_SENDER_USE_XOR,
                                          0,
                                          CONFIG_FILE_NAME );
            break;
        }
        case 2:
        {
            value = GetPrivateProfileInt( CH3_RTP_SENDER_SECTION,
                                          CH3_SENDER_USE_XOR,
                                          0,
                                          CONFIG_FILE_NAME );
            break;
        }
        case 3:
        {
            value = GetPrivateProfileInt( CH4_RTP_SENDER_SECTION,
                                          CH4_SENDER_USE_XOR,
                                          0,
                                          CONFIG_FILE_NAME );
            break;
        }
        default:
            debug_print( "invalid index %d \n", index);
            break;
    }

    return value;
}

int32 ini_get_osd_enable( const int32 index )
{
    int32 value = 0;
    switch ( index )
    {
        case 0:
        {
            value = GetPrivateProfileInt( CH1_MAIN_VIDEO_PARAM_SECTION,
                                          CH1_MAIN_VIDEO_OSD_ENABLE_KEY,
                    		              0,
                    			          CONFIG_FILE_NAME );
            break;
        }
        case 1:
        {
            value = GetPrivateProfileInt( CH1_SUB1_VIDEO_PARAM_SECTION,
                                          CH1_SUB1_VIDEO_OSD_ENABLE_KEY,
                    		              0,
                    			          CONFIG_FILE_NAME );
            break;
        }
        case 2:
        {
            value = GetPrivateProfileInt( CH2_MAIN_VIDEO_PARAM_SECTION,
                                          CH2_MAIN_VIDEO_OSD_ENABLE_KEY,
                    		              0,
                    			          CONFIG_FILE_NAME );
            break;
        }
        case 3:
        {
            value = GetPrivateProfileInt( CH2_SUB1_VIDEO_PARAM_SECTION,
                                          CH2_SUB1_VIDEO_OSD_ENABLE_KEY,
                    		              0,
                    			          CONFIG_FILE_NAME );
            break;
        }
        default:
            debug_print( "invalid index %d \n", index);
            break;
    }

    return value;
}

boolean ini_get_ftp_server( int8 *value )
{
    GetPrivateProfileString( FTP_PARAM_SECTION,
                             FTP_SERVER_URL_KEY,
				             "192.168.1.100",
				             value,
				             32,
				             CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_get_ftp_username( int8 *value )
{
    GetPrivateProfileString( FTP_PARAM_SECTION,
                             FTP_USERNAME_KEY,
				             "admin",
				             value,
				             32,
				             CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_get_ftp_password( int8 *value )
{
    GetPrivateProfileString( FTP_PARAM_SECTION,
                             FTP_PASSWORD_KEY,
				             "admin",
				             value,
				             32,
				             CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_get_ftp_directory( int8 *value )
{
    GetPrivateProfileString( FTP_PARAM_SECTION,
                             FTP_DIRECTORY_KEY,
				             "c:\\ftp_root",
				             value,
				             64,
				             CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_get_ftp_enable( void )
{
    return ( boolean )GetPrivateProfileInt( FTP_PARAM_SECTION,
                                FTP_ENABLE_KEY,
                                0,
				                CONFIG_FILE_NAME );
}

int32   ini_get_record_plan( int32 index )
{
    int8 weekday[16] = { 0 };
    sprintf( weekday, "%s%d", WEEK_DAY_KEY, index );
    return  GetPrivateProfileInt( REC_PLAN_SECTION,
                                  weekday,
                                  0,
				                  CONFIG_FILE_NAME );
}

boolean ini_get_record_enable( void )
{
    return ( boolean )GetPrivateProfileInt( REC_PLAN_SECTION,
                                            REC_ENABLE_KEY,
                                            0,
				                            CONFIG_FILE_NAME );
}

int32 ini_get_record_method( void )
{
    return GetPrivateProfileInt( REC_PLAN_SECTION,
                                 REC_METHOD_KEY,
                                 1,
				                 CONFIG_FILE_NAME );
}


boolean ini_get_ddns_server( int8 *value )
{
    GetPrivateProfileString( DDNS_SECTION,
                             DDNS_SERVER_URL_KEY,
				             "www.3322.org",
				             value,
				             32,
				             CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_get_ddns_username( int8 *value )
{
    GetPrivateProfileString( DDNS_SECTION,
                             DDNS_USERNAME_KEY,
				             "admin",
				             value,
				             32,
				             CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_get_ddns_password( int8 *value )
{
    GetPrivateProfileString( DDNS_SECTION,
                             DDNS_PASSWORD_KEY,
				             "admin",
				             value,
				             32,
				             CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_get_ddns_enable( void )
{
    return ( boolean )GetPrivateProfileInt( DDNS_SECTION,
                                            DDNS_ENABLE_KEY,
                                            0,
				                            CONFIG_FILE_NAME );
}

boolean ini_get_smtp_server( int8 *value )
{
    GetPrivateProfileString( SMTP_SECTION,
                             SMTP_SERVER1_URL_KEY,
				             "mail.126.org",
				             value,
				             32,
				             CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_get_smtp_server2( int8 *value )
{
    GetPrivateProfileString( SMTP_SECTION,
                             SMTP_SERVER2_URL_KEY,
				             "mail.126.org",
				             value,
				             32,
				             CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_get_smtp_username( int8 *value )
{
    GetPrivateProfileString( SMTP_SECTION,
                             SMTP_USERNAME_KEY,
				             "admin",
				             value,
				             32,
				             CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_get_smtp_password( int8 *value )
{
    GetPrivateProfileString( SMTP_SECTION,
                             SMTP_PASSWORD_KEY,
				             "admin",
				             value,
				             32,
				             CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_get_smtp_from_address( int8 *value )
{
    GetPrivateProfileString( SMTP_SECTION,
                             SMTP_FROM_KEY,
				             "admin@126.com",
				             value,
				             32,
				             CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_get_smtp_to_address( int8 *value )
{
    GetPrivateProfileString( SMTP_SECTION,
                             SMTP_TO1_KEY,
				             "admin@126.com",
				             value,
				             32,
				             CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_get_smtp_to_address2( int8 *value )
{
    GetPrivateProfileString( SMTP_SECTION,
                             SMTP_TO2_KEY,
				             "admin@126.com",
				             value,
				             32,
				             CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_get_smtp_enable( void )
{
    return ( boolean )GetPrivateProfileInt( SMTP_SECTION,
                                            SMTP_ENABLE_KEY,
                                            0,
				                            CONFIG_FILE_NAME );
}

int32 ini_get_upnp_internal_port( void )
{
    return GetPrivateProfileInt( UPNP_SECTION,
                                 UPNP_INTERNAL_PORT_KEY,
                                 8080,
				                 CONFIG_FILE_NAME );
}

int32 ini_get_upnp_external_port( void )
{
    return GetPrivateProfileInt( UPNP_SECTION,
                                 UPNP_EXTERNAL_PORT_KEY,
                                 8080,
				                 CONFIG_FILE_NAME );
}

boolean ini_get_upnp_enable( void )
{
    return ( boolean )GetPrivateProfileInt( UPNP_SECTION,
                                            UPNP_ENABLE_KEY,
                                            0,
				                            CONFIG_FILE_NAME );
}

int32  ini_get_ntp_time_zone( void )
{
    return  GetPrivateProfileInt( NTP_SECTION,
                                  NTP_TIME_ZONE_KEY,
                                  8,
				                  CONFIG_FILE_NAME );
}

boolean  ini_get_device_name( int8 *value )
{
    GetPrivateProfileString( DEVICE_SECTION,
                             NAME_KEY,
				             "camera0",
				             value,
				             16,
				             CONFIG_FILE_NAME );
}

////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// ini set param
/////////////////////////////////////////////////////////////
boolean ini_set_capture_res( int8 *value )
{
    int32 i;
    boolean find = FALSE;

    for ( i=0; i< ( sizeof( CameraParamTable )/sizeof(CameraParamTable[0]) ); i++ )
    {
        if ( 0 == strcmp( CameraParamTable[i].key, value ) )
        {
            find = TRUE;
            break;
        }
    }
    if ( !find )
    {
        printf("======= unsupport cap resolution %s \n", value);
        return FALSE;
    }


    WritePrivateProfileString( CH1_MAIN_CAPTURE_PARAM_SECTION,
                               CH1_MAIN_CAPTURE_RESOLUTION_KEY,
                               value,
				               CONFIG_FILE_NAME );

    return TRUE;
}

boolean ini_set_enc_res( int32 index, int8 *value )
{
    int32 i;
    boolean find = FALSE;

    for ( i=0; i< ( sizeof( CameraParamTable )/sizeof( CameraParamTable[0] ) ); i++ )
    {
        if ( 0 == strcmp( CameraParamTable[i].key, value ) )
        {
            find = TRUE;
            break;
        }
    }
    if ( !find )
    {
        printf("======= unsupport enc resolution %d:%s \n", index, value);
        return FALSE;
    }

    if ( 0 == index )
    {
        WritePrivateProfileString( CH1_MAIN_VIDEO_PARAM_SECTION,
                                   CH1_MAIN_VIDEO_RESOLUTION_KEY,
    				               value,
    				               CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        WritePrivateProfileString( CH1_SUB1_VIDEO_PARAM_SECTION,
                                   CH1_SUB1_VIDEO_RESOLUTION_KEY,
    				               value,
    				               CONFIG_FILE_NAME );
    }
    else if ( 2 == index )
    {
        WritePrivateProfileString( CH2_MAIN_VIDEO_PARAM_SECTION,
                                   CH2_MAIN_VIDEO_RESOLUTION_KEY,
    				               value,
    				               CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        WritePrivateProfileString( CH2_SUB1_VIDEO_PARAM_SECTION,
                                   CH2_SUB1_VIDEO_RESOLUTION_KEY,
    				               value,
    				               CONFIG_FILE_NAME );
    }

    return TRUE;
}

boolean ini_set_encoder( int32 index, int8 *value )
{
    if ( ( 0 != strcmp( value, "h264" ) ) && ( 0 != strcmp( value, "mjpeg" ) ) )
    {
        printf( "======= unsupport video encoder: %s \n", value );
        return FALSE;
    }

    if ( 0 == index )
    {
        WritePrivateProfileString( CH1_MAIN_VIDEO_PARAM_SECTION,
                                   CH1_MAIN_VIDEO_ENCODER_KEY,
    				               value,
    				               CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        WritePrivateProfileString( CH1_SUB1_VIDEO_PARAM_SECTION,
                                   CH1_SUB1_VIDEO_ENCODER_KEY,
                                   value,
    				               CONFIG_FILE_NAME );
    }
    else if ( 2 == index )
    {
        WritePrivateProfileString( CH2_MAIN_VIDEO_PARAM_SECTION,
                                   CH2_MAIN_VIDEO_ENCODER_KEY,
    				               value,
    				               CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        WritePrivateProfileString( CH2_SUB1_VIDEO_PARAM_SECTION,
                                   CH2_SUB1_VIDEO_ENCODER_KEY,
                                   value,
    				               CONFIG_FILE_NAME );
    }

    return TRUE;
}

boolean ini_set_framerate( int32 index, int32 value )
{
    if ( ( value <= 0 ) || ( value > 60 ) )
    {
        printf( "======= unsupport frame rate %d should[1-60] \n", value );
        return FALSE;
    }

    if ( 0 == index )
    {
        WritePrivateProfileInt( CH1_MAIN_VIDEO_PARAM_SECTION,
                                CH1_MAIN_VIDEO_FRAME_RATE_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        WritePrivateProfileInt( CH1_SUB1_VIDEO_PARAM_SECTION,
                                CH1_SUB1_VIDEO_FRAME_RATE_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if (2 == index )
    {
        WritePrivateProfileInt( CH2_MAIN_VIDEO_PARAM_SECTION,
                                CH2_MAIN_VIDEO_FRAME_RATE_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        WritePrivateProfileInt( CH2_SUB1_VIDEO_PARAM_SECTION,
                                CH2_SUB1_VIDEO_FRAME_RATE_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }

    return TRUE;
}

// 1,h.264 2,mjpeg
boolean ini_set_enc_mode( int32 index, int32 value )
{
    if ( 0 == index )
    {
        WritePrivateProfileInt( CH1_MAIN_VIDEO_PARAM_SECTION,
                                CH1_MAIN_VIDEO_ENC_MODE_KEY,
                                value,
    				            CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        WritePrivateProfileInt( CH1_SUB1_VIDEO_PARAM_SECTION,
                                CH1_SUB1_VIDEO_ENC_MODE_KEY,
                                value,
    				            CONFIG_FILE_NAME );
    }
    else if (2 == index )
    {
        WritePrivateProfileInt( CH2_MAIN_VIDEO_PARAM_SECTION,
                                CH2_MAIN_VIDEO_ENC_MODE_KEY,
                                value,
    				            CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        WritePrivateProfileInt( CH2_SUB1_VIDEO_PARAM_SECTION,
                                CH2_SUB1_VIDEO_ENC_MODE_KEY,
                                value,
    				            CONFIG_FILE_NAME );
    }

    return TRUE;
}

boolean ini_set_enc_quality( int32 index, int32 value )
{
    if ( 0 == index )
    {
        WritePrivateProfileInt( CH1_MAIN_VIDEO_PARAM_SECTION,
                                   CH1_MAIN_VIDEO_QUALITY_KEY,
                                   value,
    				               CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        WritePrivateProfileInt( CH1_SUB1_VIDEO_PARAM_SECTION,
                                   CH1_SUB1_VIDEO_QUALITY_KEY,
                                   value,
    				               CONFIG_FILE_NAME );
    }
    else if (2 == index )
    {
        WritePrivateProfileInt( CH2_MAIN_VIDEO_PARAM_SECTION,
                                   CH2_MAIN_VIDEO_QUALITY_KEY,
                                   value,
    				               CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        WritePrivateProfileInt( CH2_SUB1_VIDEO_PARAM_SECTION,
                                   CH2_SUB1_VIDEO_QUALITY_KEY,
                                   value,
    				               CONFIG_FILE_NAME );
    }

    return TRUE;
}

boolean ini_set_bitrate( int32 index, int32 value )
{
    if ( 0 == index )
    {
        WritePrivateProfileInt( CH1_MAIN_VIDEO_PARAM_SECTION,
                                CH1_MAIN_VIDEO_BITRATE_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        WritePrivateProfileInt( CH1_SUB1_VIDEO_PARAM_SECTION,
                                CH1_SUB1_VIDEO_BITRATE_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 2 == index )
    {
        WritePrivateProfileInt( CH2_MAIN_VIDEO_PARAM_SECTION,
                                CH2_MAIN_VIDEO_BITRATE_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        WritePrivateProfileInt( CH2_SUB1_VIDEO_PARAM_SECTION,
                                CH2_SUB1_VIDEO_BITRATE_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }

    return TRUE;
}

boolean ini_set_gop( int32 index, int32 value )
{
    if ( ( value <= 0 ) || ( value > 30 ) )
    {
        printf( "======= unsupport gop %d should[1-30] \n", value );
        return FALSE;
    }
    if ( 0 == index )
    {
        WritePrivateProfileInt( CH1_MAIN_VIDEO_PARAM_SECTION,
                                CH1_MAIN_VIDEO_GOP_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        WritePrivateProfileInt( CH1_SUB1_VIDEO_PARAM_SECTION,
                                CH1_SUB1_VIDEO_GOP_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 2 == index )
    {
        WritePrivateProfileInt( CH2_MAIN_VIDEO_PARAM_SECTION,
                                CH2_MAIN_VIDEO_GOP_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        WritePrivateProfileInt( CH2_SUB1_VIDEO_PARAM_SECTION,
                                CH2_SUB1_VIDEO_GOP_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }

    return TRUE;
}

boolean ini_set_osd_enable( int32 index, int32 value )
{
    if ( 0 == index )
    {
        WritePrivateProfileInt( CH1_MAIN_VIDEO_PARAM_SECTION,
                                CH1_MAIN_VIDEO_OSD_ENABLE_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        WritePrivateProfileInt( CH1_SUB1_VIDEO_PARAM_SECTION,
                                CH1_SUB1_VIDEO_OSD_ENABLE_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 2 == index )
    {
        WritePrivateProfileInt( CH2_MAIN_VIDEO_PARAM_SECTION,
                                CH2_MAIN_VIDEO_OSD_ENABLE_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        WritePrivateProfileInt( CH2_SUB1_VIDEO_PARAM_SECTION,
                                CH2_SUB1_VIDEO_OSD_ENABLE_KEY,
            		            value,
            			        CONFIG_FILE_NAME );
    }

    return TRUE;
}

boolean ini_set_xor( int32 index, int32 value )
{
    if ( 0 == index )
    {
        WritePrivateProfileInt( CH1_RTP_SENDER_SECTION,
                                CH1_SENDER_USE_XOR,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        WritePrivateProfileInt( CH2_RTP_SENDER_SECTION,
                                CH2_SENDER_USE_XOR,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 2 == index )
    {
        WritePrivateProfileInt( CH3_RTP_SENDER_SECTION,
                                CH3_SENDER_USE_XOR,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        WritePrivateProfileInt( CH4_RTP_SENDER_SECTION,
                                CH4_SENDER_USE_XOR,
            		            value,
            			        CONFIG_FILE_NAME );
    }

    return TRUE;
}

boolean ini_set_get_rec_plan( int32 index, int8 * start, int8 * end )
{
    // Start = 19:50:00
    // End   = 19:58:00
#if 0
    int8 key[16];
    memset( key, 0, sizeof( key ) );
    sprintf( key, "%s%d", START_KEY, index );
    GetPrivateProfileString( REC_PLAN_SECTION,
                             key,
				             "default",
				             start,
				             32,
				             CONFIG_FILE_NAME );

    memset( key, 0, sizeof( key ) );
    sprintf( key, "%s%d", END_KEY, index );
    GetPrivateProfileString( REC_PLAN_SECTION,
                             key,
				             "default",
				             end,
				             32,
				             CONFIG_FILE_NAME );
#endif
    return TRUE;
}

boolean ini_set_set_rec_plan( int8 * start, int8 * end )
{
    boolean ret = FALSE;

#if 0
    int32 i;
    int8  key[16];
    int8  buffer[16];
    // max item is 24?
    for( i=0; i<24; i++ )
    {
        memset( key, 0, sizeof( key ) );
        memset( buffer, 0, sizeof( buffer ) );
        sprintf( key, "%s%d", START_KEY, i );
        GetPrivateProfileString( REC_PLAN_SECTION,
                                 key,
    				             "default",
    				             buffer,
    				             16,
    				             CONFIG_FILE_NAME );
        if ( 0 == strcmp( "default", buffer ) )
        {
            memset( key, 0, sizeof( key ) );
            sprintf( key, "%s%d", START_KEY, i );
            WritePrivateProfileString( REC_PLAN_SECTION,
                                       key,
    				                   start,
    				                   CONFIG_FILE_NAME );

            memset( key, 0, sizeof( key ) );
            sprintf( key, "%s%d", END_KEY, i );
            WritePrivateProfileString( REC_PLAN_SECTION,
                                       key,
    				                   end,
    				                   CONFIG_FILE_NAME );

            debug_print( "write index %d %s - %s ok \n", i, start, end );
            ret = TRUE;
            break;
        }
    }
#endif

    return ret;
}

boolean ini_set_clear_all_rec_plan( void )
{
    int32 i;
    int8  key[16];

    // max item is 24?
    for( i=0; i<24; i++ )
    {
        memset( key, 0, sizeof( key ) );

#if 0
        sprintf( key, "%s%d", START_KEY, i );
        WritePrivateProfileString( REC_PLAN_SECTION,
                                   key,
				                   "default",
				                   CONFIG_FILE_NAME );

        memset( key, 0, sizeof( key ) );
        sprintf( key, "%s%d", END_KEY, i );
        WritePrivateProfileString( REC_PLAN_SECTION,
                                   key,
				                   "default",
				                   CONFIG_FILE_NAME );
#endif

    }

    return TRUE;
}

boolean ini_set_audio_switch( int32 index, int32 value )
{
    if ( 0 == index )
    {
        WritePrivateProfileInt( CH1_RTP_SENDER_SECTION,
                                CH1_SENDER_AUDIO_SWITCH,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 1 == index )
    {
        WritePrivateProfileInt( CH2_RTP_SENDER_SECTION,
                                CH2_SENDER_AUDIO_SWITCH,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 2 == index )
    {
        WritePrivateProfileInt( CH3_RTP_SENDER_SECTION,
                                CH3_SENDER_AUDIO_SWITCH,
            		            value,
            			        CONFIG_FILE_NAME );
    }
    else if ( 3 == index )
    {
        WritePrivateProfileInt( CH4_RTP_SENDER_SECTION,
                                CH4_SENDER_AUDIO_SWITCH,
            		            value,
            			        CONFIG_FILE_NAME );
    }

    return TRUE;
}

boolean ini_get_local_method( int8 *method )
{
    GetPrivateProfileString( LOCAL_ADDRESS_SECTION,
                             LOCAL_IP_METHOD,
                             "static",
                             method,
                             16,
                             CONFIG_FILE_NAME );
    printf("Ini get local method %s \n", method );

    return TRUE;
}

boolean ini_get_local_ip( int8 *IP )
{
    GetPrivateProfileString( LOCAL_ADDRESS_SECTION,
                             LOCAL_IP_KEY,
                             "192.168.1.13",
                             IP,
                             16,
                             CONFIG_FILE_NAME );
    printf("Ini get local ip %s \n", IP );

    return TRUE;
}

boolean ini_get_local_netmask( int8 *nm )
{
    GetPrivateProfileString( LOCAL_ADDRESS_SECTION,
                             LOCAL_NETMASK_KEY,
                             "255.255.255.0",
                             nm,
                             16,
                             CONFIG_FILE_NAME );
    printf("Ini get local nm %s \n", nm );

    return TRUE;
}

boolean ini_get_local_gateway( int8 *gw )
{
    GetPrivateProfileString( LOCAL_ADDRESS_SECTION,
                             LOCAL_GATEWAY_KEY,
                             "192.168.1.1",
                             gw,
                             16,
                             CONFIG_FILE_NAME );
    printf("Ini get local gw %s \n", gw );

    return TRUE;
}

boolean ini_get_wlan_method( int8 *method )
{
    GetPrivateProfileString( LOCAL_ADDRESS_SECTION,
                             WLAN_IP_METHOD,
                             "static",
                             method,
                             16,
                             CONFIG_FILE_NAME );
    printf("Ini get wlan method %s \n", method );

    return TRUE;
}

boolean ini_get_wlan_ip( int8 *IP )
{
    GetPrivateProfileString( LOCAL_ADDRESS_SECTION,
                             WLAN_IP_KEY,
                             "192.168.1.13",
                             IP,
                             16,
                             CONFIG_FILE_NAME );
    printf("Ini get wlan ip %s \n", IP );

    return TRUE;
}

boolean ini_get_wlan_netmask( int8 *nm )
{
    GetPrivateProfileString( LOCAL_ADDRESS_SECTION,
                             WLAN_NETMASK_KEY,
                             "255.255.255.0",
                             nm,
                             16,
                             CONFIG_FILE_NAME );
    printf("Ini get wlan nm %s \n", nm );

    return TRUE;
}

boolean ini_get_wlan_gateway( int8 *gw )
{
    GetPrivateProfileString( LOCAL_ADDRESS_SECTION,
                             WLAN_GATEWAY_KEY,
                             "192.168.1.1",
                             gw,
                             16,
                             CONFIG_FILE_NAME );
    printf("Ini get wlan gw %s \n", gw );

    return TRUE;
}

boolean ini_get_first_dns( int8 *value )
{
    GetPrivateProfileString( LOCAL_ADDRESS_SECTION,
                             FIRST_DNS,
                             "192.168.1.1",
                             value,
                             16,
                             CONFIG_FILE_NAME );
    printf("Ini get first dns %s \n", value );

    return TRUE;
}

boolean ini_get_second_dns( int8 *value )
{
    GetPrivateProfileString( LOCAL_ADDRESS_SECTION,
                             SECOND_DNS,
                             "192.168.1.3",
                             value,
                             16,
                             CONFIG_FILE_NAME );
    printf("Ini get second dns %s \n", value );

    return TRUE;
}

int32 ini_get_http_port( void )
{
    return GetPrivateProfileInt( LOCAL_ADDRESS_SECTION,
                                 HTTP_PORT,
                    		     80,
                    			 CONFIG_FILE_NAME );
}


boolean ini_set_local_method( int8 *method )
{
    WritePrivateProfileString( LOCAL_ADDRESS_SECTION,
                               LOCAL_IP_METHOD,
                               method,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_local_ip( int8 *IP )
{
    WritePrivateProfileString( LOCAL_ADDRESS_SECTION,
                               LOCAL_IP_KEY,
                               IP,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_local_netmask( int8 *nm )
{
    WritePrivateProfileString( LOCAL_ADDRESS_SECTION,
                               LOCAL_NETMASK_KEY,
                               nm,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_local_gateway( int8 *gw )
{
    WritePrivateProfileString( LOCAL_ADDRESS_SECTION,
                               LOCAL_GATEWAY_KEY,
                               gw,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_wlan_method( int8 *method )
{
    WritePrivateProfileString( LOCAL_ADDRESS_SECTION,
                               WLAN_IP_METHOD,
                               method,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_wlan_ip( int8 *IP )
{
    WritePrivateProfileString( LOCAL_ADDRESS_SECTION,
                               WLAN_IP_KEY,
                               IP,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_wlan_netmask( int8 *nm )
{
    WritePrivateProfileString( LOCAL_ADDRESS_SECTION,
                               WLAN_NETMASK_KEY,
                               nm,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_wlan_gateway( int8 *gw )
{
    WritePrivateProfileString( LOCAL_ADDRESS_SECTION,
                               WLAN_GATEWAY_KEY,
                               gw,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_first_dns( int8 *value )
{
    WritePrivateProfileString( LOCAL_ADDRESS_SECTION,
                               FIRST_DNS,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_second_dns( int8 *value )
{
    WritePrivateProfileString( LOCAL_ADDRESS_SECTION,
                               SECOND_DNS,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_http_port( int32 value )
{
    WritePrivateProfileInt( LOCAL_ADDRESS_SECTION,
                            HTTP_PORT,
                            value,
                            CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_ftp_server( int8 *value )
{
    WritePrivateProfileString( FTP_PARAM_SECTION,
                               FTP_SERVER_URL_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}
boolean ini_set_ftp_username( int8 *value )
{
    WritePrivateProfileString( FTP_PARAM_SECTION,
                               FTP_USERNAME_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_ftp_password( int8 *value )
{
    WritePrivateProfileString( FTP_PARAM_SECTION,
                               FTP_PASSWORD_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_ftp_directory( int8 *value )
{
    WritePrivateProfileString( FTP_PARAM_SECTION,
                               FTP_DIRECTORY_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_ftp_enable( boolean enable )
{
    WritePrivateProfileInt( FTP_PARAM_SECTION,
                            FTP_ENABLE_KEY,
                            enable,
                            CONFIG_FILE_NAME );
    return TRUE;
}


boolean   ini_set_record_plan( int32 index, int32 value )
{
    int8 weekday[16] = { 0 };
    sprintf( weekday, "%s%d", WEEK_DAY_KEY, index );
    WritePrivateProfileInt( REC_PLAN_SECTION,
                            weekday,
                            value,
                            CONFIG_FILE_NAME );

    return TRUE;
}

boolean ini_set_record_enable( boolean enable )
{
    WritePrivateProfileInt( REC_PLAN_SECTION,
                            REC_ENABLE_KEY,
                            enable,
                            CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_record_method( int32 value )
{
    WritePrivateProfileInt( REC_PLAN_SECTION,
                            REC_METHOD_KEY,
                            value,
				            CONFIG_FILE_NAME );
    return TRUE;
}


boolean ini_set_ddns_server( int8 *value )
{
    WritePrivateProfileString( DDNS_SECTION,
                               DDNS_SERVER_URL_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_ddns_username( int8 *value )
{
    WritePrivateProfileString( DDNS_SECTION,
                               DDNS_USERNAME_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_ddns_password( int8 *value )
{
    WritePrivateProfileString( DDNS_SECTION,
                               DDNS_PASSWORD_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}



boolean ini_set_ddns_enable( boolean enable )
{
    WritePrivateProfileInt( DDNS_SECTION,
                            DDNS_ENABLE_KEY,
                            enable,
                            CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_smtp_server( int8 *value )
{
    WritePrivateProfileString( SMTP_SECTION,
                               SMTP_SERVER1_URL_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_smtp_server2( int8 *value )
{
    WritePrivateProfileString( SMTP_SECTION,
                               SMTP_SERVER2_URL_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_smtp_username( int8 *value )
{
    WritePrivateProfileString( SMTP_SECTION,
                               SMTP_USERNAME_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_smtp_password( int8 *value )
{
    WritePrivateProfileString( SMTP_SECTION,
                               SMTP_PASSWORD_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_smtp_from_address( int8 *value )
{
    WritePrivateProfileString( SMTP_SECTION,
                               SMTP_FROM_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_smtp_to_address( int8 *value )
{
    WritePrivateProfileString( SMTP_SECTION,
                               SMTP_TO1_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_smtp_to_address2( int8 *value )
{
    WritePrivateProfileString( SMTP_SECTION,
                               SMTP_TO2_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_smtp_enable(  boolean enable )
{
    WritePrivateProfileInt( SMTP_SECTION,
                            SMTP_ENABLE_KEY,
                            enable,
				            CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_upnp_internal_port( int32 value )
{
    WritePrivateProfileInt( UPNP_SECTION,
                            UPNP_INTERNAL_PORT_KEY,
                            value,
				            CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_upnp_external_port( int32 value )
{
    WritePrivateProfileInt( UPNP_SECTION,
                            UPNP_EXTERNAL_PORT_KEY,
                            value,
				            CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_upnp_enable( boolean enable )
{
    WritePrivateProfileInt( UPNP_SECTION,
                            UPNP_ENABLE_KEY,
                            enable,
				            CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_ntp_time_zone( int32 value )
{
    WritePrivateProfileInt( NTP_SECTION,
                            NTP_TIME_ZONE_KEY,
                            value,
				            CONFIG_FILE_NAME );
    return TRUE;
}

boolean ini_set_device_name( int8 *value )
{
    WritePrivateProfileString( DEVICE_SECTION,
                               NAME_KEY,
                               value,
                               CONFIG_FILE_NAME );
    return TRUE;
}
