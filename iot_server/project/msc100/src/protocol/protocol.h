#ifndef	__PROTOCOL_H
#define	__PROTOCOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#define CLIENT_A_DYNAMIC_PWD            0x01 // pwd is random.
#define CLIENT_R_DYNAMIC_PWD            0x81

#define CLIENT_A_REGISTER               0x02
#define CLIENT_R_REGISTER               0x82

#define CLIENT_A_LOGIN                  0x03
#define CLIENT_R_LOGIN                  0x83

#define CLIENT_A_GET_DEV_LIST           0x04
#define CLIENT_R_GET_DEV_LIST           0x84

#define CLIENT_A_UPLOAD_DEV_LIST        0x06
#define CLIENT_R_UPLOAD_DEV_LIST        0x86

#define CLIENT_A_DEL_DEV_LIST           0x07
#define CLIENT_R_DEL_DEV_LIST           0x87

#define CLIENT_A_INQUIRE_DEV_STATUS     0x08
#define CLIENT_R_INQUIRE_DEV_STATUS     0x88

#define CLIENT_A_CONTROL_DEV_SWITCH     0x09
#define SERVER_A_CONTROL_DEV_SWITCH     0x0A

#define DEVICE_R_REAL_TIME_DATA         0x8A
#define SERVER_R_REAL_TIME_DATA         0x89

#define CLIENT_A_INQUIRE_DEV_INFO       0x0B // inquire: ask for.
#define CLIENT_R_INQUIRE_DEV_INFO       0x8B

#define CLIENT_A_SET_DEV_TIMER          0x0C
#define SERVER_A_SET_DEV_TIMER          0x0D

#define DEVICE_R_DEV_TIMER              0x8D
#define SERVER_R_DEV_TIMER              0x8C

#define DEVICE_A_INFORM_ONLINE          0x0E
#define SERVER_R_INFORM_ONLINE          0x8E

#define DEVICE_A_UPDATE_REAL_TIME_DATA  0x10
#define DEVICE_R_UPDATE_REAL_TIME_DATA  0x90

#define CLIENT_A_ADD_PHONE_NUM          0x11
#define CLIENT_R_ADD_PHONE_NUM          0x90

#define SERVER_A_INFORM_FLUSH           0x12

#ifdef __cplusplus
}
#endif

#endif //__PROTOCOL_H
