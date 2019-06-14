#ifndef __PROJECT_CONFIG_H
#define __PROJECT_CONFIG_H

//
#define CPU_MESSAGE                 "Linux"
#define RUN_VERSION                 "20140420-1.01"

#define DEVICE_FW_VERSION           "20140905"
#define DEVICE_TYPE                 "RT5350f"
#define DEVICE_RUN_VERSION          "201404201001"

#if 0
#define CONFIG_FILE_NAME_XML        "/var/Config_mc100.xml"
#define CONFIG_FILE_NAME0_XML       "/usr/webs/linux/Config_mc1000.xml"
#else
#define CONFIG_FILE_NAME_XML        "Config_mc100.xml"
#define CONFIG_FILE_NAME0_XML       "Config_mc1000.xml"
#endif

#define CONFIG_FILE_NAME_UM	        "/var/umconfig.txt"
#define CONFIG_FILE_NAME0_UM        "/usr/webs/linux/umconfig0.txt"

// User login section.
#define LOGIN_SECTION               "Login"
#define LOGIN_USERNAME_KEY          "Username"
#define LOGIN_PASSWORD_KEY          "Password"

// IP address section.
#define LOCAL_ADDRESS_SECTION       "Address"
#define LOCAL_IP_METHOD             "Method"
#define LOCAL_IP_KEY                "IP"
#define LOCAL_NETMASK_KEY           "Netmask"
#define LOCAL_GATEWAY_KEY           "Gateway"
#define WLAN_IP_METHOD              "WlanMethod"
#define WLAN_IP_KEY                 "WlanIP"
#define WLAN_NETMASK_KEY            "WlanNetmask"
#define WLAN_GATEWAY_KEY            "WlanGateway"
#define FIRST_DNS                   "FirstDns"
#define SECOND_DNS                  "SecondDns"
#define HTTP_PORT                   "HttpPort"

// Wireless mode section.
#define WIRELESS_WORK_PARAM_SECTION "WlanWorkParam"  // ap/sta
#define WIRELESS_MODE_KEY           "Mode"

// Wlan auth in station mode(client,station).
#define WLAN_AUTH_SECTION           "WlanAuth"
#define WLAN_ESSID_KEY              "Essid"
#define WLAN_MODE_KEY               "Mode"
#define WLAN_PASSWORD_KEY           "Password"

// Wlan AP mode.
#define WIRELESS_AP_SECTION         "WlanAP"
#define WIRELESS_AP_IP_KEY          "IP"      // 192.168.3.1 always.
#define WIRELESS_SSID_KEY           "Ssid"
#define WIRELESS_AP_ENC_KEY         "EncMode" // share/wpa
#define WIRELESS_AP_PASSWORD_KEY    "Password"

// Switch section.
#define SWITCH_SECTION              "Switch"
#define SWITCH_CH_KEY               "Channel"

// ntp section.
#define NTP_SECTION                 "Ntp"
#define NTP_TIME_ZONE_KEY           "TimeZone"

// record section.
#define REC_PLAN_SECTION            "RecordPlan"
#define WEEK_DAY_KEY                "Weekday"       // weekday0-sunday weekday1-monday weekday2
#define REC_ENABLE_KEY              "Enable"
#define REC_METHOD_KEY              "Method"

// uart param.
#define UART_SECTION                "Uart"
#define BAUD_RATE1_KEY              "BaudRate1"
#define BAUD_RATE2_KEY              "BaudRate2"
#define DATA_BITS1_KEY              "DataBits1"
#define DATA_BITS2_KEY              "DataBits2"
#define PARITY1_KEY                 "Parity1"
#define PARITY2_KEY                 "Parity2"
#define STOP_BITS1_KEY              "StopBits1"
#define STOP_BITS2_KEY              "StopBits2"

// uart udp ip address.
#define UART_UDP_SECTION            "Udp"
#define SEND_IP1_KEY                "SendIP1"
#define SEND_IP2_KEY                "SendIP2"
#define SEND_PORT1_KEY              "SendPort1"
#define SEND_PORT2_KEY              "SendPort2"
#define BIND_PORT1_KEY              "BindPort1"
#define BIND_PORT2_KEY              "BindPort2"
#define ENABLE1_KEY                 "Enable1"
#define ENABLE2_KEY                 "Enable2"

// UDP server address
#define UDP_SERVER_SECTION          "UdpServer"
#define UDP_SERVER_IP1_KEY          "IP1"
#define UDP_SERVER_IP2_KEY          "IP2"
#define UDP_SERVER_IP3_KEY          "IP3"
#define UDP_SERVER_PORT_KEY         "Port"

// device config.
#define DEVICE_SECTION              "Device"
#define NAME_KEY                    "Name"      // device name.
#define CPU_KEY                     "CPU"
#define SOFT_VERSION_KEY            "SVersion"  // software version.
#define ID_KEY                      "ID"        // id.

#endif // __PROJECT_CONFIG_H
