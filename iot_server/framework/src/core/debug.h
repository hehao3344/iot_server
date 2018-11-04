#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef  DEBUG
#define IOT_DEBUG   1
#else
#define IOT_DEBUG   0
#endif

#define filename(x) (strrchr(x,'/')?strrchr(x,'/')+1:x)

#define debug_print(fmt, ...) \
    do { if (IOT_DEBUG) fprintf(stderr, "debug,line:%d " fmt, __LINE__, ##__VA_ARGS__); } while (0)

#define debug_info(fmt, ...) \
    do { if (IOT_DEBUG) printf("%s:%d:%s(): " fmt, filename(__FILE__), __LINE__, __func__, ##__VA_ARGS__); } while (0)

#define debug_log(fmt, ...) \
	do { if (IOT_DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, filename(__FILE__), __LINE__, __func__, ##__VA_ARGS__); } while (0)

#define debug_error(fmt, ...) \
	do { if (IOT_DEBUG) fprintf(stderr, "error: %s:%d:%s(): " fmt, filename(__FILE__), __LINE__, __func__, ##__VA_ARGS__); } while (0)

#endif // __DEBUG_H
