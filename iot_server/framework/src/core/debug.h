#ifndef __DEBUG_H
#define __DEBUG_H

#include "../log/debug_util.h"

#define IOT_SERVER_PRINT

#ifdef  IOT_SERVER_PRINT
#define debug_error(fmt,args...) \
            debug_printf(IOT_DEBUG_LEVEL_ERROR, "%d [iot_server %s:%d ERROR]  "fmt, IOT_DEBUG_LEVEL_ERROR, __func__, __LINE__, ##args)
#define debug_info(fmt,args...) \
            debug_printf(IOT_DEBUG_LEVEL_INFO, "%d [iot_server %s:%d INFO] "fmt, IOT_DEBUG_LEVEL_INFO, __func__, __LINE__, ##args)
#define debug_print(fmt,args...) \
            debug_printf(IOT_DEBUG_LEVEL_INFO, "%d [iot_server %s:%d PRINT] "fmt, IOT_DEBUG_LEVEL_INFO, __func__, __LINE__, ##args)
#define debug_log(fmt,args...) \
            debug_printf(IOT_DEBUG_LEVEL_DEBUG, "%d [iot_server %s:%d DEBUG] "fmt, IOT_DEBUG_LEVEL_DEBUG, __func__, __LINE__, ##args)
#else
#define filename(x) (strrchr(x,'/')?strrchr(x,'/')+1:x)
#define debug_error(fmt,...)  do { if (1) printf("[iot_server_pr]error: %s:%s():%d " fmt, filename(__FILE__), __func__,__LINE__, ##__VA_ARGS__); } while (0)
#define debug_info(fmt,...)   do { if (1) printf("[iot_server_pr]%s:%s():%d: " fmt, filename(__FILE__),__func__, __LINE__,##__VA_ARGS__); } while (0)
#define debug_info(fmt,...)   do { if (1) printf("[iot_server_pr]%s:%s():%d: " fmt, filename(__FILE__),__func__, __LINE__,##__VA_ARGS__); } while (0)
#define debug_dbg(fmt,...)    do { if (1) printf("[iot_server_pr]%s:%s():%d: " fmt, filename(__FILE__), __func__, __LINE__, ##__VA_ARGS__); } while (0)
#endif

#if 0
#define filename(x) (strrchr(x,'/')?strrchr(x,'/')+1:x)
#define debug_print(fmt, ...) \
    do { if (IOT_DEBUG) fprintf(stderr, "debug,line:%s:%d " fmt, filename(__FILE__),__LINE__, ##__VA_ARGS__); } while (0)

#define debug_info(fmt, ...) \
    do { if (IOT_DEBUG) printf("%s:%d:%s(): " fmt, filename(__FILE__), __LINE__, __func__, ##__VA_ARGS__); } while (0)

#define debug_log(fmt, ...) \
	do { if (IOT_DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, filename(__FILE__), __LINE__, __func__, ##__VA_ARGS__); } while (0)

#define debug_error(fmt, ...) \
	do { if (IOT_DEBUG) fprintf(stderr, "error: %s:%d:%s(): " fmt, filename(__FILE__), __LINE__, __func__, ##__VA_ARGS__); } while (0)
#endif

#endif // __DEBUG_H
