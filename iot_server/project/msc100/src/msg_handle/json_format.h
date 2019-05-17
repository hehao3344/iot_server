#ifndef __JSON_FORMAT_H
#define __JSON_FORMAT_H

#ifdef __cplusplus
extern "C"
{
#endif

// 小程序 服务器
#define  JSON_IOTS_APP_BIND_RESP "{\
\"method\":\"down_msg\",\
\"open_id\":\"%s\",\
\"req_id\":%d,\
\"code\":%d\
}"

#define  JSON_IOTS_APP_GET_PARAM_RESP "{\
\"method\":\"down_msg\",\
\"dev_uuid\":\"%s\",\
\"req_id\":%d,\
\"code\":%d,\
\"attr\":\
{\
\"dev1\": \
{ \
\"dev_uuid\":\"%s\", \
\"switch\":\"%s\"\
},\
\"dev2\": \
{ \
\"dev_uuid\":\"%s\",\
\"switch\":\"%s\"\
},\
\"dev3\": \
{\
\"dev_uuid\":\"%s\",\
\"switch\":\"%s\"\
},\
\"dev4\":\
{\
\"dev_uuid\":\"%s\",\
\"switch\":\"%s\"\
}\
}\
}"

#define  JSON_IOTS_APP_HEART_BEAT_RESP "{\
\"method\":\"down_msg\",\
\"open_id\":\"%s\",\
\"req_id\":%d,\
\"code\":0\
}"

#define  JSON_IOTS_APP_SET_SWITCH_RESP "{\
\"method\":\"down_msg\", \
\"dev_uuid\":\"%s\",\
\"req_id\":%d,\
\"code\":%d\
}"


// 服务器 子设备
#define  JSON_IOTS_CC_SET_SWITCH_REQ    "{\
\"method\":\"down_msg\", \
\"cc_uuid”:\"%s\",\
\"req_id\":%d,\
\"ts\":%d,\
\"attr\":\
{\
\"dev_uuid\":\"%s\",\
\"cmd\":\"set_switch\":\
{\
\"switch\":\"%s\"\
}\
}\
}"



#ifdef __cplusplus
}
#endif

#endif

